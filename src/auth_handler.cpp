#include "auth_handler.hpp"
#include <regex>

AuthHandler::AuthHandler() {
    // Add test API key for development
    add_api_key("test-api-key-12345678901234567890123456789012");
}

bool AuthHandler::add_api_key(const std::string& api_key) {
    if (!is_valid_api_key_format(api_key)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    return valid_api_keys.insert(api_key).second;
}

bool AuthHandler::remove_api_key(const std::string& api_key) {
    std::lock_guard<std::mutex> lock(mutex);
    return valid_api_keys.erase(api_key) > 0;
}

bool AuthHandler::validate_api_key(const std::string& api_key) const {
    if (!is_valid_api_key_format(api_key)) {
        return false;
    }
    
    std::lock_guard<std::mutex> lock(mutex);
    return valid_api_keys.find(api_key) != valid_api_keys.end();
}

bool AuthHandler::is_valid_api_key_format(const std::string& api_key) {
    if (api_key.length() < MIN_API_KEY_LENGTH) {
        return false;
    }
    
    // API key should only contain alphanumeric characters and hyphens
    static const std::regex api_key_regex("^[a-zA-Z0-9-]+$");
    return std::regex_match(api_key, api_key_regex);
} 