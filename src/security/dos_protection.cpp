#include "security/dos_protection.hpp"

DosProtection::DosProtection(unsigned int max_connections, unsigned int window_seconds)
    : max_connections(max_connections), window_seconds(window_seconds) {}

bool DosProtection::allow_connection(const std::string& ip_address) {
    std::lock_guard<std::mutex> lock(connection_mutex);
    
    auto& history = connection_attempts[ip_address];
    cleanup_old_attempts(history);
    
    if (history.attempts.size() >= max_connections) {
        return false;
    }
    
    history.attempts.push(std::chrono::system_clock::now());
    return true;
}

void DosProtection::cleanup_old_attempts(ConnectionHistory& history) {
    auto now = std::chrono::system_clock::now();
    while (!history.attempts.empty()) {
        auto& oldest = history.attempts.front();
        if (std::chrono::duration_cast<std::chrono::seconds>(now - oldest).count() >= window_seconds) {
            history.attempts.pop();
        } else {
            break;
        }
    }
} 