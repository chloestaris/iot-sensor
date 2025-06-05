# IoT Sensor WebSocket API

A secure WebSocket API for IoT sensor data management with built-in security features including rate limiting, DoS protection, and permission-based access control.

## Features

- 🔐 Secure WebSocket server with API key authentication
- 🚦 Rate limiting (100 requests/60 seconds per client)
- 🛡️ DoS protection (50 connections/60 seconds per IP)
- 👥 Permission-based access control
- 📊 PostgreSQL database integration
- 🔄 Real-time sensor data processing
- 📝 JSON-based data format

## Prerequisites

- Docker and Docker Compose
- C++17 compatible compiler
- CMake 3.10 or higher
- Python 3.x (for test client)

## Quick Start

1. Clone the repository:
```bash
git clone <repository-url>
cd iot-sensor
```

2. Create and configure the environment file:
```bash
cp .env.example .env
# Edit .env with your configuration
```

3. Build and run with Docker:
```bash
docker-compose up --build
```

4. Run the test client (in a new terminal):
```bash
python3 -m venv venv
source venv/bin/activate
pip install -r requirements.txt
python test_client.py
```

## Configuration

Create a `.env` file with the following variables:

```env
# Server Configuration
WEBSOCKET_PORT=9002
MAX_CONNECTIONS=1000

# Security Settings
RATE_LIMIT_REQUESTS=100
RATE_LIMIT_WINDOW=60
DOS_MAX_CONNECTIONS=50
DOS_WINDOW=60

# Database Configuration
POSTGRES_HOST=postgres
POSTGRES_PORT=5432
POSTGRES_DB=iot_sensors
POSTGRES_USER=your_username
POSTGRES_PASSWORD=your_password

# API Keys (comma-separated)
VALID_API_KEYS=test-api-key-12345678901234567890123456789012
```

## API Documentation

### Authentication
```json
{
    "api_key": "your-api-key"
}
```

### Sensor Data Format
```json
{
    "sensor_data": {
        "sensor_id": "temp_sensor_001",
        "type": "temperature",
        "value": 23.5,
        "timestamp": 1234567890,
        "unit": "celsius",
        "metadata": {
            "location": "room1",
            "calibration_date": "2024-01-01"
        }
    }
}
```

### Supported Sensor Types
- temperature (celsius)
- humidity (percent)
- pressure (pascal)
- light (lux)

## Security Features

### Rate Limiting
- Default: 100 requests per 60 seconds per client
- Configurable via environment variables

### DoS Protection
- Default: 50 connections per 60 seconds per IP
- Configurable via environment variables

### Permission Levels
- READ_SENSOR: Read sensor data
- WRITE_SENSOR: Send sensor data
- MANAGE_SENSORS: Add/remove sensors
- ADMIN: Full access

## Development

### Building Locally

```bash
mkdir build && cd build
cmake ..
make
```

### Running Tests

```bash
cd build
ctest
```

## Docker Support

The project includes:
- `Dockerfile` for the main application
- `docker-compose.yml` for orchestrating the application and database

## Contributing

1. Fork the repository
2. Create your feature branch
3. Commit your changes
4. Push to the branch
5. Create a Pull Request

## License

[Your License Here] 