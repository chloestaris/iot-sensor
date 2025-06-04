# IoT Sensor WebSocket API

A WebSocket server for IoT sensor data collection with API key authentication.

## Features

- Fast WebSocket communication
- API key authentication
- JSON-based sensor data format
- Support for various sensor types and units
- Real-time data validation
- Thread-safe implementation

## Prerequisites

- CMake (>= 3.10)
- C++17 compatible compiler
- OpenSSL
- Boost
- nlohmann-json

### Installing Dependencies

#### macOS
```bash
brew install openssl boost nlohmann-json
```

#### Ubuntu/Debian
```bash
sudo apt-get install libssl-dev libboost-all-dev nlohmann-json3-dev
```

## Building

1. Clone the repository:
```bash
git clone <repository-url>
cd iot-sensor
```

2. Create a build directory:
```bash
mkdir build && cd build
```

3. Configure and build:
```bash
cmake ..
make
```

## Running the Server

```bash
./iot_sensor_api
```

The server will start listening on port 9002.

## API Usage

### Authentication

Connect to the WebSocket server (ws://localhost:9002) and send an authentication message:

```json
{
    "api_key": "your-api-key-here"
}
```

### Sending Sensor Data

After authentication, send sensor readings in the following format:

```json
{
    "sensor_data": {
        "sensor_id": "sensor123",
        "type": "temperature",
        "value": 23.5,
        "timestamp": 1634567890,
        "unit": "celsius",
        "metadata": {
            "location": "room1",
            "calibration_date": "2023-01-01"
        }
    }
}
```

### Supported Sensor Types

- temperature
- humidity
- pressure
- light
- motion
- sound
- air_quality
- voltage
- current

### Supported Units

- celsius
- fahrenheit
- kelvin
- percent
- pascal
- hpa
- lux
- db
- volt
- ampere
- ppm

## Error Handling

The server will respond with error messages in the following format:

```json
{
    "error": "Error message here"
}
```

Common error types:
- Invalid API key
- Not authenticated
- Invalid JSON format
- Invalid sensor data format
- Invalid sensor type or unit

## Security Considerations

- API keys must be at least 32 characters long
- Only alphanumeric characters and hyphens are allowed in API keys
- For production use, it's recommended to set up TLS/SSL termination using a reverse proxy

## License

MIT License 