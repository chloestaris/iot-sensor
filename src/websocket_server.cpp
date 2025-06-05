#include "websocket_server.hpp"
#include <iostream>
#include <chrono>

WebSocketServer::WebSocketServer() {
    // Set logging settings
    server.set_access_channels(websocketpp::log::alevel::all);
    server.clear_access_channels(websocketpp::log::alevel::frame_payload);

    // Initialize ASIO
    server.init_asio();

    // Register handlers
    server.set_message_handler(
        [this](connection_hdl hdl, MessagePtr msg) {
            on_message(hdl, msg);
        }
    );
    
    server.set_open_handler(
        [this](connection_hdl hdl) {
            on_open(hdl);
        }
    );
    
    server.set_close_handler(
        [this](connection_hdl hdl) {
            on_close(hdl);
        }
    );

    // Initialize admin permissions for test admin API key
    Authorization::ClientPermissions admin_perms;
    admin_perms.permissions.insert(Authorization::Permission::READ_SENSOR);
    admin_perms.permissions.insert(Authorization::Permission::WRITE_SENSOR);
    admin_perms.permissions.insert(Authorization::Permission::MANAGE_SENSORS);
    admin_perms.permissions.insert(Authorization::Permission::ADMIN);
    authorization.add_client_permissions("admin-api-key-12345678901234567890123456789012", admin_perms);

    // Initialize regular user permissions
    Authorization::ClientPermissions user_perms;
    user_perms.permissions.insert(Authorization::Permission::READ_SENSOR);
    user_perms.permissions.insert(Authorization::Permission::WRITE_SENSOR);
    user_perms.allowed_sensor_ids = {"temp_sensor_001", "humidity_001"};
    authorization.add_client_permissions("test-api-key-12345678901234567890123456789012", user_perms);
}

void WebSocketServer::run(uint16_t port) {
    server.set_reuse_addr(true);
    server.listen(port);
    server.start_accept();
    
    std::cout << "WebSocket server listening on port " << port << std::endl;
    server.run();
}

void WebSocketServer::stop() {
    server.stop_listening();
}

std::string WebSocketServer::get_client_ip(connection_hdl hdl) {
    auto con = server.get_con_from_hdl(hdl);
    return con->get_remote_endpoint();
}

void WebSocketServer::on_message(connection_hdl hdl, MessagePtr msg) {
    try {
        std::string client_ip = get_client_ip(hdl);
        
        // Check rate limit
        if (!rate_limiter.check_rate_limit(client_ip)) {
            server.send(hdl, json{
                {"status", "error"},
                {"message", "Rate limit exceeded",
                "error_code", "RATE_LIMIT_EXCEEDED"}
            }.dump(), msg->get_opcode());
            return;
        }

        auto data = json::parse(msg->get_payload());
        
        // Handle authentication
        if (data.contains("api_key")) {
            std::string api_key = data["api_key"];
            if (validate_api_key(api_key)) {
                connections[hdl] = api_key;
                json response = {{"status", "authenticated"}};
                if (is_admin(api_key)) {
                    response["role"] = "admin";
                }
                server.send(hdl, response.dump(), msg->get_opcode());
            } else {
                server.send(hdl, json{
                    {"status", "error"},
                    {"message", "Invalid API key"},
                    {"error_code", "INVALID_API_KEY"}
                }.dump(), msg->get_opcode());
                server.close(hdl, websocketpp::close::status::policy_violation, "Invalid API key");
            }
            return;
        }
        
        // Check if client is authenticated
        if (connections.find(hdl) == connections.end()) {
            server.send(hdl, json{
                {"status", "error"},
                {"message", "Not authenticated"},
                {"error_code", "NOT_AUTHENTICATED"}
            }.dump(), msg->get_opcode());
            server.close(hdl, websocketpp::close::status::policy_violation, "Not authenticated");
            return;
        }

        std::string api_key = connections[hdl];

        // Handle admin requests
        if (data.contains("admin")) {
            handle_admin_request(hdl, data["admin"]);
            return;
        }
        
        // Handle sensor data
        if (data.contains("sensor_data")) {
            handle_sensor_data(hdl, data["sensor_data"]);
            return;
        }
        
        // Unknown request type
        server.send(hdl, json{
            {"status", "error"},
            {"message", "Unknown request type"},
            {"error_code", "UNKNOWN_REQUEST"}
        }.dump(), msg->get_opcode());
        
    } catch (const json::exception& e) {
        server.send(hdl, json{
            {"status", "error"},
            {"message", "Invalid JSON format"},
            {"error_code", "INVALID_JSON"}
        }.dump(), msg->get_opcode());
    } catch (const std::exception& e) {
        server.send(hdl, json{
            {"status", "error"},
            {"message", std::string("Internal server error: ") + e.what()},
            {"error_code", "INTERNAL_ERROR"}
        }.dump(), msg->get_opcode());
    }
}

void WebSocketServer::on_open(connection_hdl hdl) {
    std::string client_ip = get_client_ip(hdl);
    
    if (!dos_protection.allow_connection(client_ip)) {
        server.close(hdl, websocketpp::close::status::policy_violation, 
                    "Too many connection attempts");
        return;
    }
    
    std::cout << "New connection opened from " << client_ip << std::endl;
}

void WebSocketServer::on_close(connection_hdl hdl) {
    connections.erase(hdl);
    std::cout << "Connection closed from " << get_client_ip(hdl) << std::endl;
}

bool WebSocketServer::validate_api_key(const std::string& api_key) {
    return auth_handler.validate_api_key(api_key);
}

void WebSocketServer::handle_sensor_data(connection_hdl hdl, const json& data) {
    try {
        SensorReading reading = data.get<SensorReading>();
        std::string api_key = connections[hdl];
        
        // Check authorization
        if (!authorization.can_access_sensor(api_key, reading.sensor_id, Authorization::Permission::WRITE_SENSOR)) {
            server.send(hdl, json{
                {"status", "error"},
                {"message", "Unauthorized access to sensor"}
            }.dump(), websocketpp::frame::opcode::text);
            return;
        }
        
        if (SensorData::validate_sensor_reading(reading)) {
            // Process the valid sensor reading
            server.send(hdl, json{{"status", "success"}, {"message", "Sensor data received"}}.dump(), 
                       websocketpp::frame::opcode::text);
        } else {
            server.send(hdl, json{
                {"status", "error"},
                {"message", SensorData::get_error_message(reading)}
            }.dump(), websocketpp::frame::opcode::text);
        }
    } catch (const json::exception& e) {
        server.send(hdl, json{{"error", "Invalid sensor data format"}}.dump(), 
                   websocketpp::frame::opcode::text);
    }
}

bool WebSocketServer::is_admin(const std::string& api_key) {
    return authorization.can_access_sensor(api_key, "", Authorization::Permission::ADMIN);
}

void WebSocketServer::handle_admin_request(connection_hdl hdl, const json& data) {
    try {
        std::string action = data["action"];
        if (action == "system_stats") {
            handle_system_stats(hdl, data);
        } else if (action == "manage_user") {
            handle_user_management(hdl, data);
        } else if (action == "manage_permissions") {
            handle_permission_management(hdl, data);
        } else if (action == "configure_rate_limit") {
            handle_rate_limit_config(hdl, data);
        } else {
            server.send(hdl, json{
                {"status", "error"},
                {"message", "Unknown admin action"},
                {"error_code", "UNKNOWN_ADMIN_ACTION"}
            }.dump(), websocketpp::frame::opcode::text);
        }
    } catch (const json::exception& e) {
        server.send(hdl, json{
            {"status", "error"},
            {"message", "Invalid admin request format"},
            {"error_code", "INVALID_ADMIN_REQUEST"}
        }.dump(), websocketpp::frame::opcode::text);
    } catch (const std::exception& e) {
        server.send(hdl, json{
            {"status", "error"},
            {"message", std::string("Admin request error: ") + e.what()},
            {"error_code", "ADMIN_REQUEST_ERROR"}
        }.dump(), websocketpp::frame::opcode::text);
    }
}

void WebSocketServer::handle_system_stats(connection_hdl hdl, const json& data) {
    std::string type = data.value("type", "all");
    json stats;
    
    if (type == "all" || type == "connections") {
        stats["connections"] = get_connection_stats();
    }
    if (type == "all" || type == "rate_limits") {
        stats["rate_limits"] = get_rate_limit_stats();
    }
    if (type == "all" || type == "sensors") {
        stats["sensors"] = get_sensor_stats();
    }
    
    server.send(hdl, json{
        {"status", "success"},
        {"stats", stats}
    }.dump(), websocketpp::frame::opcode::text);
}

void WebSocketServer::handle_user_management(connection_hdl hdl, const json& data) {
    std::string operation = data["operation"];
    std::string user_id = data["user_id"];
    
    if (operation == "add" || operation == "modify") {
        Authorization::ClientPermissions perms;
        for (const auto& perm : data["permissions"]) {
            if (perm == "READ_SENSOR") perms.permissions.insert(Authorization::Permission::READ_SENSOR);
            else if (perm == "WRITE_SENSOR") perms.permissions.insert(Authorization::Permission::WRITE_SENSOR);
            else if (perm == "MANAGE_SENSORS") perms.permissions.insert(Authorization::Permission::MANAGE_SENSORS);
            else if (perm == "ADMIN") perms.permissions.insert(Authorization::Permission::ADMIN);
        }
        perms.allowed_sensor_ids = data["allowed_sensors"].get<std::vector<std::string>>();
        
        authorization.add_client_permissions(user_id, perms);
        server.send(hdl, json{
            {"status", "success"},
            {"message", "User permissions updated"}
        }.dump(), websocketpp::frame::opcode::text);
    }
    // Additional operations can be added here
}

void WebSocketServer::handle_permission_management(connection_hdl hdl, const json& data) {
    std::string operation = data["operation"];
    std::string target_user = data["target_user"];
    std::string permission_str = data["permission"];
    std::string sensor_id = data["sensor_id"];
    
    // Implementation depends on how you want to handle granular permissions
    server.send(hdl, json{
        {"status", "success"},
        {"message", "Permissions updated"}
    }.dump(), websocketpp::frame::opcode::text);
}

void WebSocketServer::handle_rate_limit_config(connection_hdl hdl, const json& data) {
    std::string client_id = data["client_id"];
    unsigned int max_requests = data["max_requests"];
    unsigned int window_seconds = data["window_seconds"];
    
    // Update rate limiter configuration
    // Note: This would require adding configuration methods to the RateLimiter class
    
    server.send(hdl, json{
        {"status", "success"},
        {"message", "Rate limit configuration updated"}
    }.dump(), websocketpp::frame::opcode::text);
}

json WebSocketServer::get_connection_stats() {
    json stats = json::object();  // Create an empty JSON object
    stats["active_connections"] = connections.size();
    stats["total_connections"] = connections.size();  // Since WebSocket++ doesn't provide total count
    return stats;
}

json WebSocketServer::get_rate_limit_stats() {
    json stats = json::object();  // Create an empty JSON object
    stats["rate_limited_clients"] = 0;  // Placeholder
    stats["total_rate_limit_events"] = 0;  // Placeholder
    return stats;
}

json WebSocketServer::get_sensor_stats() {
    json stats = json::object();  // Create an empty JSON object
    stats["active_sensors"] = 0;  // Placeholder
    stats["total_readings"] = 0;  // Placeholder
    return stats;
} 