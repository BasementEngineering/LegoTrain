import './style.css'
import javascriptLogo from './javascript.svg'
import viteLogo from '/vite.svg'
import { setupCounter } from './counter.js'
const socket = new WebSocket('ws://your-websocket-url');

const NO_BACKEND = true;
var chart = null;

function setupDataGenerator() {
  setInterval(() => {
    updateValues();
  }, 100)
}

function setupChart() {
  const ctx = document.getElementById('chart').getContext('2d');
  chart = new Chart(ctx, config);
}

var currentSpeed = 0;
var currentOutput = 0;
var currentSetpoint = 0;
var timeStamp = 0;

const config = {
  type: 'line',
  data: {
    labels: [],
    datasets: [
      {
        label: 'Setpoint',
        data: [],
        borderColor: 'red',
        fill: false
      },
      {
        label: 'Input',
        data: [],
        borderColor: 'blue',
        fill: false
      },
      {
        label: 'Output',
        data: [],
        borderColor: 'green',
        fill: false
      }
    ]
  },
  options: {
    responsive: true,
    scales: {
      x: {
        display: true,
        title: {
          display: true,
          text: 'Time'
        }
      },
      y: {
        display: true,
        title: {
          display: true,
          text: 'Value'
        }
      }
    }
  }
};

function addData(chart, label, data) {
  if (chart.data.labels.length >= 50) {
    chart.data.labels.shift();
    chart.data.datasets.forEach((dataset) => {
      dataset.data.shift();
    });
  }
  chart.data.labels.push(label);
  chart.data.datasets.forEach((dataset) => {
    dataset.data.push(data[chart.data.datasets.indexOf(dataset)]);
  });
  chart.update();
}

function updateValues() {
  currentSpeed = Math.floor(Math.random() * 100);
  currentOutput = Math.floor(Math.random() * 100);
  currentSetpoint = Math.floor(Math.random() * 100);
  timeStamp += 0.1;
  timeStamp = parseFloat(timeStamp.toFixed(1));
  addData(chart, timeStamp, [currentSetpoint, currentSpeed, currentOutput]);
}

function main() {
  setupChart();

  if (NO_BACKEND) {
    setupDataGenerator();
  }
  else {
  socket.addEventListener('message', function (event) {
    const data = event.data.split(',');
    currentSpeed = parseFloat(data[0]);
    currentOutput = parseFloat(data[1]);
    currentSetpoint = parseFloat(data[2]);
    timeStamp += 0.1;
    timeStamp = parseFloat(timeStamp.toFixed(1));
    addData(chart, timeStamp, [currentSetpoint, currentSpeed, currentOutput]);
  });
  }
  
  
  }

main();