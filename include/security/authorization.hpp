#pragma once

#include <string>
#include <set>
#include <vector>
#include <map>
#include <mutex>

class Authorization {
public:
    enum class Permission {
        READ_SENSOR,
        WRITE_SENSOR,
        MANAGE_SENSORS,
        ADMIN
    };

    struct ClientPermissions {
        std::set<Permission> permissions;
        std::vector<std::string> allowed_sensor_ids;
    };

    bool add_client_permissions(const std::string& client_id, const ClientPermissions& permissions);
    bool can_access_sensor(const std::string& client_id, const std::string& sensor_id, Permission required_permission);

private:
    std::map<std::string, ClientPermissions> client_permissions;
    std::mutex permissions_mutex;
}; 