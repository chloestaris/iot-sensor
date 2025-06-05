#pragma once

#include <string>
#include <map>
#include <mutex>
#include <chrono>

class RateLimiter {
public:
    RateLimiter(unsigned int max_requests = 100, unsigned int window_seconds = 60);
    bool check_rate_limit(const std::string& client_id);

private:
    struct ClientQuota {
        unsigned int requests;
        std::chrono::system_clock::time_point window_start;
    };

    std::map<std::string, ClientQuota> client_quotas;
    std::mutex quota_mutex;
    unsigned int max_requests;
    unsigned int window_seconds;

    void cleanup_old_entries();
}; 