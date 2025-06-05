#!/usr/bin/env python3

import asyncio
import websockets
import json
import time
from datetime import datetime

# Test configuration
SERVER_URL = "ws://localhost:9002"
ADMIN_API_KEY = "admin-api-key-12345678901234567890123456789012"

async def send_message(websocket, message):
    await websocket.send(json.dumps(message))
    response = await websocket.recv()
    return json.loads(response)

async def run_admin_tests():
    try:
        print(f"Connecting to {SERVER_URL}...")
        async with websockets.connect(SERVER_URL) as websocket:
            # Test 1: Admin Authentication
            print("\n1. Testing Admin Authentication")
            auth_response = await send_message(websocket, {
                "api_key": ADMIN_API_KEY
            })
            print(f"Authentication response: {auth_response}")
            
            if "error" in auth_response:
                print("Authentication failed. Stopping tests.")
                return

            # Test 2: Get System Stats
            print("\n2. Testing System Stats")
            stats_request = {
                "admin": {
                    "action": "system_stats",
                    "type": "all"
                }
            }
            stats_response = await send_message(websocket, stats_request)
            print(f"System stats response: {stats_response}")

            # Test 3: User Management - Add User
            print("\n3. Testing User Management - Add User")
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
            print(f"Add user response: {user_response}")

            # Test 4: Permission Management
            print("\n4. Testing Permission Management")
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

            # Test 5: Rate Limit Configuration
            print("\n5. Testing Rate Limit Configuration")
            rate_limit_request = {
                "admin": {
                    "action": "configure_rate_limit",
                    "client_id": "new_user_123",
                    "max_requests": 200,
                    "window_seconds": 60
                }
            }
            rate_limit_response = await send_message(websocket, rate_limit_request)
            print(f"Rate limit configuration response: {rate_limit_response}")

    except websockets.exceptions.ConnectionRefusedError:
        print("Error: Could not connect to the server. Make sure it's running.")
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    print("IoT Sensor API Admin Test Client")
    print("================================")
    asyncio.run(run_admin_tests()) 