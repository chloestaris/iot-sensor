cmake_minimum_required(VERSION 3.10)
project(iot_sensor_api)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find required packages
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(Boost REQUIRED COMPONENTS system)
find_package(PostgreSQL REQUIRED)

# Find libpqxx
find_library(PQXX_LIB pqxx)
find_library(PQ_LIB pq)

# Add WebSocket++ headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Add source files
set(SOURCES
    src/main.cpp
    src/websocket_server.cpp
    src/auth_handler.cpp
    src/sensor_data.cpp
    src/security/rate_limiter.cpp
    src/security/authorization.cpp
    src/security/dos_protection.cpp
)

# Create executable
add_executable(${PROJECT_NAME} ${SOURCES})

# Link libraries
target_link_libraries(${PROJECT_NAME}
    PRIVATE
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    Boost::system
    PostgreSQL::PostgreSQL
    ${PQXX_LIB}
    ${PQ_LIB}
)

# Include directories
target_include_directories(${PROJECT_NAME}
    PRIVATE
    ${CMAKE_SOURCE_DIR}/include
    ${PostgreSQL_INCLUDE_DIRS}
) 