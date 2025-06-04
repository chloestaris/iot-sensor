#!/usr/bin/env python3

import asyncio
import websockets
import json
import time
from datetime import datetime
import sys

# Test configuration
SERVER_URL = "ws://localhost:9002"
API_KEY = "test-api-key-12345678901234567890123456789012"  # 32 chars minimum

async def send_message(websocket, message):
    await websocket.send(json.dumps(message))
    response = await websocket.recv()
    return json.loads(response)

async def run_tests():
    try:
        print(f"Connecting to {SERVER_URL}...")
        async with websockets.connect(SERVER_URL) as websocket:
            # Test 1: Authentication
            print("\n1. Testing Authentication")
            auth_response = await send_message(websocket, {
                "api_key": API_KEY
            })
            print(f"Authentication response: {auth_response}")
            
            if "error" in auth_response:
                print("Authentication failed. Stopping tests.")
                return

            # Test 2: Send valid temperature reading
            print("\n2. Testing Valid Temperature Reading")
            temp_data = {
                "sensor_data": {
                    "sensor_id": "temp_sensor_001",
                    "type": "temperature",
                    "value": 23.5,
                    "timestamp": int(time.time()),
                    "unit": "celsius",
                    "metadata": {
                        "location": "room1",
                        "calibration_date": "2024-01-01"
                    }
                }
            }
            temp_response = await send_message(websocket, temp_data)
            print(f"Temperature data response: {temp_response}")

            # Test 3: Send valid humidity reading
            print("\n3. Testing Valid Humidity Reading")
            humidity_data = {
                "sensor_data": {
                    "sensor_id": "humidity_001",
                    "type": "humidity",
                    "value": 45.2,
                    "timestamp": int(time.time()),
                    "unit": "percent",
                    "metadata": {
                        "location": "room1"
                    }
                }
            }
            humidity_response = await send_message(websocket, humidity_data)
            print(f"Humidity data response: {humidity_response}")

            # Test 4: Send invalid sensor type
            print("\n4. Testing Invalid Sensor Type")
            invalid_type_data = {
                "sensor_data": {
                    "sensor_id": "test_001",
                    "type": "invalid_type",
                    "value": 100,
                    "timestamp": int(time.time()),
                    "unit": "percent"
                }
            }
            invalid_type_response = await send_message(websocket, invalid_type_data)
            print(f"Invalid type response: {invalid_type_response}")

            # Test 5: Send invalid unit
            print("\n5. Testing Invalid Unit")
            invalid_unit_data = {
                "sensor_data": {
                    "sensor_id": "temp_002",
                    "type": "temperature",
                    "value": 23.5,
                    "timestamp": int(time.time()),
                    "unit": "invalid_unit"
                }
            }
            invalid_unit_response = await send_message(websocket, invalid_unit_data)
            print(f"Invalid unit response: {invalid_unit_response}")

    except websockets.exceptions.ConnectionRefusedError:
        print("Error: Could not connect to the server. Make sure it's running.")
    except Exception as e:
        print(f"Error: {str(e)}")

if __name__ == "__main__":
    print("IoT Sensor API Test Client")
    print("=========================")
    asyncio.get_event_loop().run_until_complete(run_tests()) 