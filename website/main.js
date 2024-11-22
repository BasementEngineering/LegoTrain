import './style.css'
import { initModeSelctor,getChoosenMode,setChoosenMode,setUpdateCallback } from './modeSelector';
import { initTrainControls, getSetpointValue } from './trainControls';

var controlData ={
  setpoint: 0,
  input: 0,
  output: 0
}

const maxSpeed = 500;

const NO_BACKEND = false;

let websocket;
const host = window.location.hostname;
//const host = "192.168.178.94";

function parseWsMessage(event) {
    const message = JSON.parse(event.data);
    console.log('WebSocket message:', message);

    if (message.hasOwnProperty('controlData')) {
      controlData = message.controlData;
      updateBars(controlData);
    }
}

function initWebSocket() {
    websocket = new WebSocket('ws://' + host + ':81/');
    websocket.onopen = () => console.log('WebSocket connection opened');
    websocket.onclose = () => {
        console.log('WebSocket connection closed, attempting to reconnect');
        setTimeout(initWebSocket, 1000);
    };
    websocket.onerror = (error) => console.error('WebSocket error:', error);
    websocket.onmessage = parseWsMessage;
}

function onSelectionChanged(mode) {
    console.log('onSelectionChanged', mode);
    fetch(`http://${host}/mode`, {
      method: 'POST',
      headers: {
        'Content-Type': 'application/json'
      },
      body: JSON.stringify({ mode })
    })
    .then(response => response.json())
    .then(data => console.log('Mode updated:', data))
    .catch(error => console.error('Error updating mode:', error));
}

function sendJoystickValue(value) {
    console.log(JSON.stringify({ setpoint: value }));
    if (websocket && websocket.readyState === WebSocket.OPEN) {
        websocket.send(JSON.stringify({ setpoint: value }));
    }
}

function genrateData(){
  controlData.setpoint = getSetpointValue();

  controlData.input = controlData.setpoint + ( 5 - Math.floor(Math.random() * 10));

  if(controlData.setpoint > controlData.input){
    controlData.output = controlData.output + 1;
  }
  else if(controlData.setpoint < controlData.input){
    controlData.output = controlData.output - 1;
  }
  if(controlData.output > 100){
    controlData.output = 100;
  }

}

function updateBars(controlData){

  var setpointPercentage = controlData.setpoint;
  var inputPercentage = controlData.input;
  if(getChoosenMode() == 2 || getChoosenMode() == 3){
    setpointPercentage = (controlData.setpoint * 100) / maxSpeed;
    inputPercentage = (controlData.input * 100) / maxSpeed;
  }

  document.getElementById('bar_setpoint').style.height = `${50 + 0.5*setpointPercentage}%`;
  document.getElementById('bar_setpoint').innerHTML = controlData.setpoint;

  document.getElementById('bar_input').style.height = `${50+ 0.5*inputPercentage}%`;
  document.getElementById('bar_input').innerHTML = controlData.input;

  document.getElementById('bar_output').style.height = `${50+ 0.5*controlData.output}%`;
  document.getElementById('bar_output').innteHTML = controlData.output;
}

function initFromBackendData(){
  fetch(`http://${host}/mode`)
    .then(response => response.json())
    .then(data => setChoosenMode(data.mode))
    .catch(error => console.error('Error fetching mode:', error));
}

function main(){
  initWebSocket();
  initModeSelctor();
  setUpdateCallback(onSelectionChanged);

  initTrainControls();

  setInterval(() => {
    
    if(NO_BACKEND){
      genrateData();
    }
    else{
      sendJoystickValue(getSetpointValue());
    }
    updateBars(controlData);

}, 100);

  if(!NO_BACKEND){
    initFromBackendData();
  }
}

main();