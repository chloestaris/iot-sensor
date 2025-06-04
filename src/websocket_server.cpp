#include "websocket_server.hpp"
#include <iostream>

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

void WebSocketServer::on_message(connection_hdl hdl, MessagePtr msg) {
    try {
        auto data = json::parse(msg->get_payload());
        
        // Check if message contains API key for authentication
        if (data.contains("api_key")) {
            std::string api_key = data["api_key"];
            if (validate_api_key(api_key)) {
                connections[hdl] = api_key;
                server.send(hdl, json{{"status", "authenticated"}}.dump(), msg->get_opcode());
            } else {
                server.send(hdl, json{{"error", "Invalid API key"}}.dump(), msg->get_opcode());
                server.close(hdl, websocketpp::close::status::policy_violation, "Invalid API key");
            }
            return;
        }
        
        // Check if client is authenticated
        if (connections.find(hdl) == connections.end()) {
            server.send(hdl, json{{"error", "Not authenticated"}}.dump(), msg->get_opcode());
            server.close(hdl, websocketpp::close::status::policy_violation, "Not authenticated");
            return;
        }
        
        // Handle sensor data
        if (data.contains("sensor_data")) {
            handle_sensor_data(hdl, data["sensor_data"]);
        }
        
    } catch (const json::exception& e) {
        server.send(hdl, json{{"error", "Invalid JSON format"}}.dump(), msg->get_opcode());
    }
}

void WebSocketServer::on_open(connection_hdl hdl) {
    std::cout << "New connection opened" << std::endl;
}

void WebSocketServer::on_close(connection_hdl hdl) {
    connections.erase(hdl);
    std::cout << "Connection closed" << std::endl;
}

bool WebSocketServer::validate_api_key(const std::string& api_key) {
    return auth_handler.validate_api_key(api_key);
}

void WebSocketServer::handle_sensor_data(connection_hdl hdl, const json& data) {
    try {
        SensorReading reading = data.get<SensorReading>();
        
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