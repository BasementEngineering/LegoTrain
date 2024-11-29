from flask import Flask, request, jsonify
import threading
import time

app = Flask(__name__)
filling_state = False

def turn_off_filling_state():
    global filling_state
    time.sleep(10)
    filling_state = False

@app.route('/fill', methods=['POST', 'GET'])
def fill():
    global filling_state
    if request.method == 'POST':
        filling_state = True
        threading.Thread(target=turn_off_filling_state).start()
        return jsonify({"message": "Filling started"}), 200
    elif request.method == 'GET':
        return jsonify({"filling_state": filling_state}), 200

if __name__ == '__main__':
    app.run(host='192.168.178.78', port=80)