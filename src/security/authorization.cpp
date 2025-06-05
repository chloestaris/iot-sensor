#include "security/authorization.hpp"
#include <algorithm>

bool Authorization::add_client_permissions(const std::string& client_id, const ClientPermissions& permissions) {
    std::lock_guard<std::mutex> lock(permissions_mutex);
    client_permissions[client_id] = permissions;
    return true;
}

bool Authorization::can_access_sensor(const std::string& client_id, 
                                   const std::string& sensor_id,
                                   Permission required_permission) {
    std::lock_guard<std::mutex> lock(permissions_mutex);
    
    auto it = client_permissions.find(client_id);
    if (it == client_permissions.end()) {
        return false;
    }
    
    const auto& perms = it->second;
    
    // Check if client has admin permission
    if (perms.permissions.find(Permission::ADMIN) != perms.permissions.end()) {
        return true;
    }
    
    // Check if client has the required permission
    if (perms.permissions.find(required_permission) == perms.permissions.end()) {
        return false;
    }
    
    // Check if client has access to the specific sensor
    return std::find(perms.allowed_sensor_ids.begin(),
                    perms.allowed_sensor_ids.end(),
                    sensor_id) != perms.allowed_sensor_ids.end();
} 