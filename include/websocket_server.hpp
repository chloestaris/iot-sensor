#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include "auth_handler.hpp"
#include "sensor_data.hpp"
#include "security/rate_limiter.hpp"
#include "security/authorization.hpp"
#include "security/dos_protection.hpp"

using json = nlohmann::json;
using websocketpp::connection_hdl;

class WebSocketServer {
public:
    using Server = websocketpp::server<websocketpp::config::asio>;
    using MessagePtr = Server::message_ptr;

    WebSocketServer();
    void run(uint16_t port);
    void stop();

private:
    Server server;
    AuthHandler auth_handler;
    RateLimiter rate_limiter;
    Authorization authorization;
    DosProtection dos_protection;
    std::map<connection_hdl, std::string, std::owner_less<connection_hdl>> connections;
    
    // Message handlers
    void on_message(connection_hdl hdl, MessagePtr msg);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    
    // Authentication
    bool validate_api_key(const std::string& api_key);
    
    // Data handlers
    void handle_sensor_data(connection_hdl hdl, const json& data);
    std::string get_client_ip(connection_hdl hdl);

    // Admin handlers
    void handle_admin_request(connection_hdl hdl, const json& data);
    void handle_system_stats(connection_hdl hdl, const json& data);
    void handle_user_management(connection_hdl hdl, const json& data);
    void handle_permission_management(connection_hdl hdl, const json& data);
    void handle_rate_limit_config(connection_hdl hdl, const json& data);
    
    // Helper methods
    bool is_admin(const std::string& api_key);
    json get_connection_stats();
    json get_rate_limit_stats();
    json get_sensor_stats();
}; 