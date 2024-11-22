const char htmlPage[] PROGMEM = R"rawliteral(
<!DOCTYPE html>
<html lang="en">
<head>
    <meta charset="UTF-8">
    <meta name="viewport" content="width=device-width, initial-scale=1.0">
    <title>Remote Train Control</title>
  <script type="module" crossorigin>(function(){const t=document.createElement("link").relList;if(t&&t.supports&&t.supports("modulepreload"))return;for(const o of document.querySelectorAll('link[rel="modulepreload"]'))i(o);new MutationObserver(o=>{for(const n of o)if(n.type==="childList")for(const u of n.addedNodes)u.tagName==="LINK"&&u.rel==="modulepreload"&&i(u)}).observe(document,{childList:!0,subtree:!0});function c(o){const n={};return o.integrity&&(n.integrity=o.integrity),o.referrerPolicy&&(n.referrerPolicy=o.referrerPolicy),o.crossOrigin==="use-credentials"?n.credentials="include":o.crossOrigin==="anonymous"?n.credentials="omit":n.credentials="same-origin",n}function i(o){if(o.ep)return;o.ep=!0;const n=c(o);fetch(o.href,n)}})();var d=["btn_mode1","btn_mode2","btn_mode3","btn_mode4"],l=0,a=null;function _(){console.log("initModeSelctor"),d.forEach(function(e){document.getElementById(e).addEventListener("click",function(){b(e)})}),k(0)}function b(e){console.log("onModeButtonClicked"),l=d.indexOf(e),d.forEach(function(t){a!=null&&a(l),t==e?(console.log("Secondary: "+t),document.getElementById(t).style="background-color: var(--secondary-color)"):(console.log("Deactivated: "+t),document.getElementById(t).style="background-color: var(--deactivated-color)")})}function h(){return l}function k(e){b(d[e])}function C(e){a=e}const s=document.getElementById("joystick"),g=document.querySelector(".joystick-container");function B(){s.style.top=`${g.offsetHeight-s.offsetHeight}px`}var f=["btn_direction_forwards","btn_direction_backwards"],S=0;function O(){f.forEach(function(e){document.getElementById(e).addEventListener("click",function(){y(e)})}),document.getElementById("stop_btn").addEventListener("click",function(){B()}),y("btn_direction_forwards")}function y(e){console.log("onDirectionButtonClicked"),S=f.indexOf(e),f.forEach(function(t){t==e?document.getElementById(t).style="background-color: var(--secondary-color)":document.getElementById(t).style="background-color: var(--deactivated-color)"})}function I(){window.addEventListener("load",B),s.addEventListener("touchstart",e=>{e.preventDefault();const t=c=>{v(c.touches[0])};document.addEventListener("touchmove",t),document.addEventListener("touchend",()=>{document.removeEventListener("touchmove",t)},{once:!0})}),s.addEventListener("mousedown",()=>{const e=t=>{v(t)};document.addEventListener("mousemove",e),document.addEventListener("mouseup",()=>{document.removeEventListener("mousemove",e)},{once:!0})}),O()}function w(){var e=P();return S==1&&(e=-e),e}function P(){const e=g.getBoundingClientRect(),t=s.getBoundingClientRect(),c=e.top+e.height/2,i=t.top+t.height/2,o=e.height/2-t.height/2,n=i-c;return Math.round(51-n/o*50)}function v(e){const t=g.getBoundingClientRect(),c=t.height-s.offsetHeight;let i=e.clientY-t.top-s.offsetHeight/2;i=Math.max(0,Math.min(c,i)),s.style.top=`${i}px`}var m={setpoint:0,input:0,output:0};const E=500;let r;const p=window.location.hostname;function j(e){const t=JSON.parse(e.data);console.log("WebSocket message:",t),t.hasOwnProperty("controlData")&&(m=t.controlData,M(m))}function L(){r=new WebSocket("ws://"+p+":81/"),r.onopen=()=>console.log("WebSocket connection opened"),r.onclose=()=>{console.log("WebSocket connection closed, attempting to reconnect"),setTimeout(L,1e3)},r.onerror=e=>console.error("WebSocket error:",e),r.onmessage=j}function T(e){console.log("onSelectionChanged",e),fetch(`http://${p}/mode`,{method:"POST",headers:{"Content-Type":"application/json"},body:JSON.stringify({mode:e})}).then(t=>t.json()).then(t=>console.log("Mode updated:",t)).catch(t=>console.error("Error updating mode:",t))}function x(e){console.log(JSON.stringify({setpoint:e})),r&&r.readyState===WebSocket.OPEN&&r.send(JSON.stringify({setpoint:e}))}function M(e){var t=e.setpoint,c=e.input;(h()==2||h()==3)&&(t=e.setpoint*100/E,c=e.input*100/E),document.getElementById("bar_setpoint").style.height=`${50+.5*t}%`,document.getElementById("bar_setpoint").innerHTML=e.setpoint,document.getElementById("bar_input").style.height=`${50+.5*c}%`,document.getElementById("bar_input").innerHTML=e.input,document.getElementById("bar_output").style.height=`${50+.5*e.output}%`,document.getElementById("bar_output").innteHTML=e.output}function J(){fetch(`http://${p}/mode`).then(e=>e.json()).then(e=>k(e.mode)).catch(e=>console.error("Error fetching mode:",e))}function N(){L(),_(),C(T),I(),setInterval(()=>{x(w()),M(m)},100),J()}N();</script>
  <style rel="stylesheet" crossorigin>:root{--primary-color: #cbcdd8a2;--secondary-color: #ffae00;--secondary-color-shade1: #ff9100;--secondary-color-shade2: #ff6200;--secondary-color-shade3: #ff5100;--deactivated-color: #c9b68e}body{display:flex;justify-content:center;align-items:center;height:100vh;margin:0;font-family:Arial,sans-serif;background-color:var(--primary-color)}.container{display:flex;flex-direction:column;width:100%;height:100%}.top-half{display:flex;flex-direction:column;width:100%;height:50%}.buttons{display:flex;justify-content:space-around;align-items:center;height:30%}.mode-button{width:20%;height:80%;background-color:var(--secondary-color);color:#fff;border:none;border-radius:5px;font-size:1.2em;cursor:pointer}.bars{display:flex;justify-content:space-around;align-items:center;height:70%}.bar{width:20%;height:80%;background-color:#f0f0f0;border:2px solid #ccc;border-radius:10px;position:relative}.setpoint-value{width:100%;height:20%;background-color:var(--secondary-color-shade1);position:absolute;bottom:0;border-radius:10px 10px 0 0}.input-value{width:100%;height:20%;background-color:var(--secondary-color-shade2);position:absolute;bottom:0;border-radius:10px 10px 0 0}.output-value{width:100%;height:20%;background-color:var(--secondary-color-shade3);position:absolute;bottom:0;border-radius:10px 10px 0 0}.output-range{width:80%;margin:10px 0;accent-color:var(--secondary-color)}.bottom-left-quarter{display:flex;justify-content:center;align-items:center;width:50%;height:50%;position:absolute;bottom:0;left:0}.bottom-right-quarter{display:flex;justify-content:center;align-items:center;flex-direction:column;width:50%;height:50%;position:absolute;bottom:0;right:0}.bottom-right-button{width:80%;height:20%;background-color:var(--secondary-color);color:#fff;border:none;border-radius:5px;font-size:1.2em;cursor:pointer}.joystick-container{width:100px;height:300px;background-color:#f0f0f0;border:2px solid #ccc;border-radius:10px;position:relative}.joystick{width:80px;height:80px;background-color:var(--secondary-color);border-radius:50%;position:absolute;left:50%;transform:translate(-50%);cursor:pointer}</style>
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
                <div class="bar">
                    <div class="setpoint-value" id="bar_setpoint"></div>
                </div>
                <div class="bar">
                    <div class="input-value" id="bar_input"></div>
                </div>
                <div class="bar">
                    <div class="output-value" id="bar_output"></div>
                </div>
            </div>
        </div>
        <div class="bottom-left-quarter">
            <div class="joystick-container">
                <div class="joystick" id="joystick"></div>
            </div>
        </div>
        <div class="bottom-right-quarter">
            <!--
          <label for="max_speed">Max Speed</label>
            <input type="range" class="output-range" id="range_max_speed" min="0" max="500" value="300">
            <p id="max_speed_value">0</p>
            <label for="max_output">Max Output</label>
            <input type="range" class="output-range" id="range_max_output" min="0" max="100" value="80">
            <p id="max_output_value">0</p>-->
            <button class="bottom-right-button" id="btn_direction_forwards">Forwards</button>
            <button class="bottom-right-button" id="btn_direction_backwards">Backwards</button>
            <button class="bottom-right-button" style="background-color: red;" id="stop_btn">Staap</button>

        </div>
    </div>
</body>
</html>
)rawliteral";