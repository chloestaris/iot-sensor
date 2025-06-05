#include "auth_handler.hpp"
#include <regex>
#include <cstdlib>
#include <sstream>

AuthHandler::AuthHandler() {
    // Get API keys from environment variable
    const char* env_api_keys = std::getenv("VALID_API_KEYS");
    if (env_api_keys) {
        std::string api_keys_str(env_api_keys);
        std::stringstream ss(api_keys_str);
        std::string api_key;
        
        // Split by comma
        while (std::getline(ss, api_key, ',')) {
            // Trim whitespace
            api_key.erase(0, api_key.find_first_not_of(" \t\n\r\f\v"));
            api_key.erase(api_key.find_last_not_of(" \t\n\r\f\v") + 1);
            
            if (!api_key.empty()) {
                add_api_key(api_key);
            }
        }
    } else {
        // Fallback to default test keys
        add_api_key("test-api-key-12345678901234567890123456789012");  // Regular user
        add_api_key("admin-api-key-12345678901234567890123456789012"); // Admin user
    }
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