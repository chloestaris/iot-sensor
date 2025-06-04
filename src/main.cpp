#include "websocket_server.hpp"
#include <iostream>
#include <csignal>

WebSocketServer* server_ptr = nullptr;

void signal_handler(int signal) {
    if (server_ptr) {
        std::cout << "\nShutting down server..." << std::endl;
        server_ptr->stop();
    }
}

int main() {
    try {
        // Set up signal handling
        server_ptr = new WebSocketServer();
        std::signal(SIGINT, signal_handler);
        std::signal(SIGTERM, signal_handler);
        
        // Run server on port 9002
        const uint16_t port = 9002;
        std::cout << "Starting IoT Sensor WebSocket server..." << std::endl;
        server_ptr->run(port);
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    delete server_ptr;
    return 0;
} 