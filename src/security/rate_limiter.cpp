#include "security/rate_limiter.hpp"

RateLimiter::RateLimiter(unsigned int max_requests, unsigned int window_seconds)
    : max_requests(max_requests), window_seconds(window_seconds) {}

bool RateLimiter::check_rate_limit(const std::string& client_id) {
    std::lock_guard<std::mutex> lock(quota_mutex);
    cleanup_old_entries();
    
    auto now = std::chrono::system_clock::now();
    auto& quota = client_quotas[client_id];
    
    if (quota.requests == 0 || 
        std::chrono::duration_cast<std::chrono::seconds>(now - quota.window_start).count() >= window_seconds) {
        quota.requests = 1;
        quota.window_start = now;
        return true;
    }
    
    if (quota.requests >= max_requests) {
        return false;
    }
    
    quota.requests++;
    return true;
}

void RateLimiter::cleanup_old_entries() {
    auto now = std::chrono::system_clock::now();
    for (auto it = client_quotas.begin(); it != client_quotas.end();) {
        if (std::chrono::duration_cast<std::chrono::seconds>(now - it->second.window_start).count() >= window_seconds) {
            it = client_quotas.erase(it);
        } else {
            ++it;
        }
    }
} 