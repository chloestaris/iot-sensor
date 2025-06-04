#pragma once

#include <websocketpp/config/asio_no_tls.hpp>
#include <websocketpp/server.hpp>
#include <nlohmann/json.hpp>
#include "auth_handler.hpp"
#include "sensor_data.hpp"

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
    std::map<connection_hdl, std::string, std::owner_less<connection_hdl>> connections;
    
    void on_message(connection_hdl hdl, MessagePtr msg);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    bool validate_api_key(const std::string& api_key);
    void handle_sensor_data(connection_hdl hdl, const json& data);
}; 