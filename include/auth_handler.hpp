#pragma once

#include <string>
#include <unordered_set>
#include <mutex>

class AuthHandler {
public:
    AuthHandler();
    
    bool add_api_key(const std::string& api_key);
    bool remove_api_key(const std::string& api_key);
    bool validate_api_key(const std::string& api_key) const;
    
private:
    mutable std::mutex mutex;
    std::unordered_set<std::string> valid_api_keys;
    
    static constexpr size_t MIN_API_KEY_LENGTH = 32;
    static bool is_valid_api_key_format(const std::string& api_key);
}; 