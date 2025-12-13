"""
Train Station Sound Player
A Flask web server that handles sound playbook for train station events.
"""

import os
import random
import json
import multiprocessing
from flask import Flask, request, jsonify
import pyttsx3
import threading
import time
from pathlib import Path

from ttsTest import TTSProcessManager

app = Flask(__name__)
# Initialize TTS manager as None initially
tts_manager = None

def init_tts():
    """Initialize TTS manager"""
    global tts_manager
    if tts_manager is None:
        tts_manager = TTSProcessManager()

@app.route('/trainArriving', methods=['POST', 'GET'])
def train_arriving():
    """Play a random train arriving sound"""
    init_tts()  # Ensure TTS is initialized
    
    trainArrivingMessages = [
    "Ihr Zug fährt ein, bitte nicht mehr auf den Gleisen schlafen",
    "Achtung, der Zug kommt",
    "Achtung, dieser Zug hat keine Bremsen",
    "Auf Gleis 1 fährt in Kürze der RE69 ein",
    "Zurücktreten bitte, Alkohol ist im Anmarsch",
    "Achtung, die Party ein",
    "Heute statt Personen, Alkohol im Gleis"
    ]
    try:
        # Select a random message
        message = random.choice(trainArrivingMessages)
        
        # Use TTS to speak the message
        tts_manager.speak(message)
        
        return jsonify({
            "status": "success",
            "message": f"Playing train arriving sound: {message}"
        })       
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": f"Error playing train arriving sound: {str(e)}"
        }), 500

@app.route('/orderReady', methods=['POST'])
def order_ready():
    """Play delivery ready announcement followed by names and colors"""
    init_tts()  # Ensure TTS is initialized
    
    try:
        # Parse JSON request
        data = request.get_json()
        
        if not data:
            return jsonify({
                "status": "error",
                "message": "No JSON data provided"
            }), 400
        
        nr_of_pairs = 0
        pairs = []
        for i in range(1, 10):
            name_key = f"name{i}"
            color_key = f"color{i}"
            if name_key in data and color_key in data:
                name = data[name_key]
                color = data[color_key]
                pairs.append((name, color))
                nr_of_pairs += 1
            else:
                break

        order_intro = f"In diesem Zug finden Sie {nr_of_pairs} shotgläser."
        order_message = ""
        for pair in pairs:
            order_message += f"Eins für {pair[0]} in {pair[1]}, "
        order_message = order_message.strip().rstrip(',')

        tts_manager.speak(order_intro)
        tts_manager.speak(order_message)
            
        return jsonify({
                "status": "success",
                "message": f"Playing delivery announcement for {len(pairs)} pairs",
                "pairs": pairs,
                "method": "audio files"
            })
            
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": f"Error processing order ready request: {str(e)}"
        }), 500

@app.route('/orderReadyTTS', methods=['POST'])
def order_ready_tts():
    """Alternative endpoint that always uses text-to-speech"""
    try:
        # Parse JSON request
        data = request.get_json()
        
        if not data:
            return jsonify({
                "status": "error",
                "message": "No JSON data provided"
            }), 400
        
        # Force TTS mode
        data['use_tts'] = True
        
        # Reuse the main order_ready logic
        return order_ready()
            
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": f"Error processing TTS order ready request: {str(e)}"
        }), 500

@app.route('/test', methods=['GET'])
def test_endpoint():
    """Test endpoint to verify the server is running"""
    return jsonify({
        "status": "success",
        "message": "Train station sound server is running",
        "endpoints": [
            "/trainArriving - Play random train arriving sound",
            "/orderReady - Play delivery announcement with audio files or TTS",
            "/orderReadyTTS - Play delivery announcement with TTS only",
            "/test - This test endpoint"
        ]
    })

@app.route('/status', methods=['GET'])
def status():
    """Get status information about available sound files"""
    try:
        status_info = {
            "tts_available": True
        }
        
        return jsonify({
            "status": "success",
            "sound_status": status_info
        })
        
    except Exception as e:
        return jsonify({
            "status": "error",
            "message": f"Error getting status: {str(e)}"
        }), 500

if __name__ == '__main__':
    multiprocessing.freeze_support()  # Required for Windows multiprocessing
    
    print("Train Station Sound Server Starting...")
    print("Available endpoints:")
    print("  GET/POST /trainArriving - Play random train arriving sound")
    print("  POST /orderReady - Play delivery announcement")
    print("  POST /orderReadyTTS - Play delivery announcement with TTS")
    print("  GET /test - Test endpoint")
    print("  GET /status - Get sound file status")
    
    try:
        # Run the Flask app
        app.run(host='0.0.0.0', port=5000, debug=True)
    finally:
        # Cleanup TTS resources on shutdown
        if tts_manager:
            print("Cleaning up TTS resources...")
            tts_manager.shutdown()