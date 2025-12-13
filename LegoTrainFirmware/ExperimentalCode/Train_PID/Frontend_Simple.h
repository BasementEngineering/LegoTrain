// Simplified frontend with external files approach
#ifndef FRONTEND_H
#define FRONTEND_H

// Much smaller, simpler HTML that loads external files
const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Train Control</title>
    <link rel="stylesheet" href="/style.css">
</head>
<body>
    <div class="container">
        <div class="top-half">
            <div class="buttons">
                <button class="mode-button" id="btn_mode1">Manual</button>
                <button class="mode-button" id="btn_mode2">Target Output</button>
                <button class="mode-button" id="btn_mode3">Target Speed</button>
                <button class="mode-button" id="btn_mode4">Full Auto</button>
            </div>
            <div class="bars">
                <div class="bar"><div class="setpoint-value" id="bar_setpoint"></div></div>
                <div class="bar"><div class="input-value" id="bar_input"></div></div>
                <div class="bar"><div class="output-value" id="bar_output"></div></div>
            </div>
        </div>
        <div class="bottom-left-quarter">
            <div class="joystick-container">
                <div class="joystick" id="joystick"></div>
            </div>
        </div>
        <div class="bottom-right-quarter">
            <button class="bottom-right-button" id="btn_direction_forwards">Forward</button>
            <button class="bottom-right-button" id="btn_direction_backwards">Backward</button>
            <button class="bottom-right-button" id="stop_btn">STOP</button>
        </div>
    </div>
    <script src="/app.js"></script>
</body>
</html>
)rawliteral";

#endif