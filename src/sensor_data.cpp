#include "sensor_data.hpp"
#include <regex>
#include <unordered_set>
#include <cmath>

void to_json(nlohmann::json& j, const SensorReading& reading) {
    j = nlohmann::json{
        {"sensor_id", reading.sensor_id},
        {"type", reading.type},
        {"value", reading.value},
        {"timestamp", std::chrono::system_clock::to_time_t(reading.timestamp)},
        {"unit", reading.unit}
    };
    
    if (!reading.metadata.is_null()) {
        j["metadata"] = reading.metadata;
    }
}

void from_json(const nlohmann::json& j, SensorReading& reading) {
    j.at("sensor_id").get_to(reading.sensor_id);
    j.at("type").get_to(reading.type);
    j.at("value").get_to(reading.value);
    reading.timestamp = std::chrono::system_clock::from_time_t(j.at("timestamp").get<time_t>());
    j.at("unit").get_to(reading.unit);
    
    if (j.contains("metadata")) {
        reading.metadata = j["metadata"];
    }
}

bool SensorData::validate_sensor_reading(const SensorReading& reading) {
    return is_valid_sensor_id(reading.sensor_id) &&
           is_valid_type(reading.type) &&
           is_valid_value(reading.value) &&
           is_valid_unit(reading.unit);
}

std::string SensorData::get_error_message(const SensorReading& reading) {
    if (!is_valid_sensor_id(reading.sensor_id)) {
        return "Invalid sensor ID format";
    }
    if (!is_valid_type(reading.type)) {
        return "Invalid sensor type";
    }
    if (!is_valid_value(reading.value)) {
        return "Invalid sensor value";
    }
    if (!is_valid_unit(reading.unit)) {
        return "Invalid measurement unit";
    }
    return "Valid sensor reading";
}

bool SensorData::is_valid_sensor_id(const std::string& sensor_id) {
    static const std::regex sensor_id_regex("^[a-zA-Z0-9-_]+$");
    return std::regex_match(sensor_id, sensor_id_regex);
}

bool SensorData::is_valid_type(const std::string& type) {
    static const std::unordered_set<std::string> valid_types = {
        "temperature", "humidity", "pressure", "light", "motion",
        "sound", "air_quality", "voltage", "current"
    };
    return valid_types.find(type) != valid_types.end();
}

bool SensorData::is_valid_value(double value) {
    return !std::isnan(value) && !std::isinf(value);
}

bool SensorData::is_valid_unit(const std::string& unit) {
    static const std::unordered_set<std::string> valid_units = {
        "celsius", "fahrenheit", "kelvin", "percent", "pascal",
        "hpa", "lux", "db", "volt", "ampere", "ppm"
    };
    return valid_units.find(unit) != valid_units.end();
} 