#!/usr/bin/env python3

import asyncio
import websockets
import json
import time
from datetime import datetime
import sys

# Test configuration
SERVER_URL = "ws://localhost:9002"
ADMIN_API_KEY = "admin-api-key-12345678901234567890123456789012"
REGULAR_API_KEY = "test-api-key-12345678901234567890123456789012"
INVALID_API_KEY = "invalid-api-key-12345678901234567890123456789012"

async def send_message(websocket, message):
    await websocket.send(json.dumps(message))
    response = await websocket.recv()
    return json.loads(response)

async def test_authentication(websocket, api_key, expected_role=None):
    print(f"\nTesting Authentication with {'admin' if api_key == ADMIN_API_KEY else 'regular'} API key")
    auth_response = await send_message(websocket, {
        "api_key": api_key
    })
    print(f"Authentication response: {auth_response}")
    
    if "error" in auth_response:
        return False
        
    if expected_role:
        if expected_role == "admin" and auth_response.get("role") != "admin":
            print("Error: Expected admin role but didn't receive it")
            return False
        elif expected_role == "regular" and "role" in auth_response:
            print("Error: Regular user received admin role")
            return False
            
    return True

async def test_admin_endpoints(websocket, api_key, should_succeed=True):
    print(f"\nTesting Admin Endpoints (expected to {'succeed' if should_succeed else 'fail'})")
    
    # Test system stats
    print("\n1. Testing System Stats Access")
    stats_request = {
        "admin": {
            "action": "system_stats",
            "type": "all"
        }
    }
    stats_response = await send_message(websocket, stats_request)
    print(f"System stats response: {stats_response}")
    if should_succeed and "error" in stats_response:
        print("Error: Admin access to system stats failed")
    elif not should_succeed and "error" not in stats_response:
        print("Error: Non-admin access to system stats succeeded")

    # Test user management
    print("\n2. Testing User Management Access")
    add_user_request = {
        "admin": {
            "action": "manage_user",
            "operation": "add",
            "user_id": "new_user_123",
            "permissions": ["READ_SENSOR", "WRITE_SENSOR"],
            "allowed_sensors": ["temp_sensor_001", "humidity_001"]
        }
    }
    user_response = await send_message(websocket, add_user_request)
    print(f"User management response: {user_response}")
    if should_succeed and "error" in user_response:
        print("Error: Admin access to user management failed")
    elif not should_succeed and "error" not in user_response:
        print("Error: Non-admin access to user management succeeded")

    # Test permission management
    print("\n3. Testing Permission Management Access")
    permission_request = {
        "admin": {
            "action": "manage_permissions",
            "operation": "grant",
            "target_user": "new_user_123",
            "permission": "MANAGE_SENSORS",
            "sensor_id": "temp_sensor_001"
        }
    }
    permission_response = await send_message(websocket, permission_request)
    print(f"Permission management response: {permission_response}")
    if should_succeed and "error" in permission_response:
        print("Error: Admin access to permission management failed")
    elif not should_succeed and "error" not in permission_response:
        print("Error: Non-admin access to permission management succeeded")

async def test_regular_endpoints(websocket):
    print("\nTesting Regular User Endpoints")
    
    # Test sensor data access
    print("\n1. Testing Sensor Data Access")
    sensor_request = {
        "sensor_data": {
            "sensor_id": "temp_sensor_001",
            "type": "temperature",
            "value": 23.5,
            "timestamp": int(time.time()),
            "unit": "celsius",
            "metadata": {
                "location": "room1"
            }
        }
    }
    sensor_response = await send_message(websocket, sensor_request)
    print(f"Sensor data response: {sensor_response}")

async def run_security_tests():
    try:
        print("Starting Security Tests")
        print("=======================")

        # Test 1: Invalid API Key
        print("\nTest 1: Invalid API Key")
        async with websockets.connect(SERVER_URL) as websocket:
            success = await test_authentication(websocket, INVALID_API_KEY)
            if success:
                print("Error: Authentication succeeded with invalid API key")

        # Test 2: Regular User Authentication and Access
        print("\nTest 2: Regular User Tests")
        async with websockets.connect(SERVER_URL) as websocket:
            if await test_authentication(websocket, REGULAR_API_KEY, "regular"):
                # Test regular endpoints
                await test_regular_endpoints(websocket)
                # Try to access admin endpoints (should fail)
                await test_admin_endpoints(websocket, REGULAR_API_KEY, should_succeed=False)

        # Test 3: Admin User Authentication and Access
        print("\nTest 3: Admin User Tests")
        async with websockets.connect(SERVER_URL) as websocket:
            if await test_authentication(websocket, ADMIN_API_KEY, "admin"):
                # Test admin endpoints
                await test_admin_endpoints(websocket, ADMIN_API_KEY, should_succeed=True)
                # Test regular endpoints (admin should have access)
                await test_regular_endpoints(websocket)

    except websockets.exceptions.ConnectionRefusedError:
        print("Error: Could not connect to the server. Make sure it's running.")
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    print("IoT Sensor API Security Test Client")
    print("===================================")
    asyncio.run(run_security_tests()) 