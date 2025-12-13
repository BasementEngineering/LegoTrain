"""
Test script for the Train Station Sound Player
"""

import requests
import json
import time

BASE_URL = "http://localhost:5000"

def test_server():
    """Test if the server is running"""
    try:
        response = requests.get(f"{BASE_URL}/test")
        print("Server test:", response.json())
        return True
    except requests.exceptions.ConnectionError:
        print("Server is not running. Please start main.py first.")
        return False

def test_train_arriving():
    """Test the train arriving endpoint"""
    print("\n--- Testing Train Arriving ---")
    try:
        response = requests.post(f"{BASE_URL}/trainArriving")
        print("Response:", response.json())
    except Exception as e:
        print("Error:", e)

def test_order_ready_files():
    """Test order ready with audio files"""
    print("\n--- Testing Order Ready (Audio Files) ---")
    
    order_data = {
        "name1": "Oliver",
        "color1": "green",
        "name2": "Emma", 
        "color2": "blue",
        "name3": "Max",
        "color3": "red",
        "use_tts": False
    }
    
    try:
        response = requests.post(f"{BASE_URL}/orderReady", json=order_data)
        print("Response:", response.json())
    except Exception as e:
        print("Error:", e)

def test_order_ready_tts():
    """Test order ready with text-to-speech"""
    print("\n--- Testing Order Ready (Text-to-Speech) ---")
    
    order_data = {
        "name1": "Alice",
        "color1": "purple",
        "name2": "Bob", 
        "color2": "yellow",
        "use_tts": True
    }
    
    try:
        response = requests.post(f"{BASE_URL}/orderReady", json=order_data)
        print("Response:", response.json())
    except Exception as e:
        print("Error:", e)

def test_status():
    """Test the status endpoint"""
    print("\n--- Testing Status ---")
    try:
        response = requests.get(f"{BASE_URL}/status")
        print("Status:", json.dumps(response.json(), indent=2))
    except Exception as e:
        print("Error:", e)

if __name__ == "__main__":
    print("Train Station Sound Player Test Script")
    print("=" * 50)
    
    if not test_server():
        exit(1)
    
    # Run tests
    test_status()
    test_train_arriving()
    
    # Wait a moment
    time.sleep(1)
    
    test_order_ready_files()
    
    # Wait a moment
    time.sleep(2)
    
    test_order_ready_tts()
    
    print("\n" + "=" * 50)
    print("Test completed!")