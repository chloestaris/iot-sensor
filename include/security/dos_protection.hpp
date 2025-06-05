#pragma once

#include <string>
#include <map>
#include <queue>
#include <mutex>
#include <chrono>

class DosProtection {
public:
    DosProtection(unsigned int max_connections = 50, unsigned int window_seconds = 60);
    bool allow_connection(const std::string& ip_address);

private:
    struct ConnectionHistory {
        std::queue<std::chrono::system_clock::time_point> attempts;
    };

    std::map<std::string, ConnectionHistory> connection_attempts;
    std::mutex connection_mutex;
    unsigned int max_connections;
    unsigned int window_seconds;

    void cleanup_old_attempts(ConnectionHistory& history);
}; 