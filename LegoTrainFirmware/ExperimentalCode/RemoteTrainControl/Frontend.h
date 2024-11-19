const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Remote Train Control</title>
    <style>
        body {
            display: flex;
            justify-content: center;
            align-items: center;
            height: 100vh;
            margin: 0;
            font-family: Arial, sans-serif;
        }
        .joystick-container {
            width: 100px;
            height: 300px;
            background-color: #f0f0f0;
            border: 2px solid #ccc;
            border-radius: 10px;
            position: relative;
        }
        .joystick {
            width: 80px;
            height: 80px;
            background-color: #007bff;
            border-radius: 50%;
            position: absolute;
            left: 50%;
            transform: translateX(-50%);
            cursor: pointer;
        }
    </style>
</head>
<body>
    <div class="joystick-container">
        <div class="joystick" id="joystick"></div>
    </div>

    <script>
        const joystick = document.getElementById('joystick');
        const container = document.querySelector('.joystick-container');
        let websocket;
        const host = window.location.hostname;

        function initWebSocket() {
            websocket = new WebSocket('ws://' + host + ':81/');
            websocket.onopen = () => console.log('WebSocket connection opened');
            websocket.onclose = () => {
                console.log('WebSocket connection closed, attempting to reconnect');
                setTimeout(initWebSocket, 1000);
            };
            websocket.onerror = (error) => console.error('WebSocket error:', error);
        }

        function sendJoystickValue(value) {
            console.log(JSON.stringify({ speed: value }));
            if (websocket && websocket.readyState === WebSocket.OPEN) {
                websocket.send(JSON.stringify({ speed: value }));
            }
        }
        function resetJoystickPosition() {
            joystick.style.top = `${container.offsetHeight / 2 - joystick.offsetHeight / 2}px`;
        }

        window.addEventListener('load', resetJoystickPosition);
        
        function getJoystickValue() {
            const rect = container.getBoundingClientRect();
            const joystickRect = joystick.getBoundingClientRect();
            const centerY = rect.top + rect.height / 2;
            const joystickCenterY = joystickRect.top + joystickRect.height / 2;
            const maxDistance = rect.height / 2 - joystickRect.height / 2;
            const distance = joystickCenterY - centerY;
            return Math.round((distance / maxDistance) * 100);
        }

        function updateJoystickPosition(event) {
            const rect = container.getBoundingClientRect();
            const maxTop = rect.height - joystick.offsetHeight;
            let top = event.clientY - rect.top - joystick.offsetHeight / 2;
            top = Math.max(0, Math.min(maxTop, top));
            joystick.style.top = `${top}px`;
        }
        joystick.addEventListener('touchstart', (event) => {
            event.preventDefault();
            const onTouchMove = (event) => {
            updateJoystickPosition(event.touches[0]);
            //sendJoystickValue(getJoystickValue());
            };

            document.addEventListener('touchmove', onTouchMove);
            document.addEventListener('touchend', () => {
            document.removeEventListener('touchmove', onTouchMove);
            }, { once: true });
        });
        joystick.addEventListener('mousedown', () => {
            const onMouseMove = (event) => {
                updateJoystickPosition(event);
                //sendJoystickValue(getJoystickValue());
            };

            document.addEventListener('mousemove', onMouseMove);
            document.addEventListener('mouseup', () => {
                document.removeEventListener('mousemove', onMouseMove);
            }, { once: true });
        });

        setInterval(() => {
            sendJoystickValue(getJoystickValue());
        }, 100);

        initWebSocket();
    </script>
</body>
</html>
)rawliteral";