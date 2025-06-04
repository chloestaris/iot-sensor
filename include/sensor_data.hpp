#pragma once

#include <string>
#include <chrono>
#include <nlohmann/json.hpp>

struct SensorReading {
    std::string sensor_id;
    std::string type;
    double value;
    std::chrono::system_clock::time_point timestamp;
    std::string unit;
    
    // Optional metadata
    nlohmann::json metadata;
};

// JSON serialization
void to_json(nlohmann::json& j, const SensorReading& reading);
void from_json(const nlohmann::json& j, SensorReading& reading);

class SensorData {
public:
    static bool validate_sensor_reading(const SensorReading& reading);
    static std::string get_error_message(const SensorReading& reading);
    
private:
    static bool is_valid_sensor_id(const std::string& sensor_id);
    static bool is_valid_type(const std::string& type);
    static bool is_valid_value(double value);
    static bool is_valid_unit(const std::string& unit);
}; 