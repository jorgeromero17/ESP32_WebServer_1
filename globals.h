//Se llama las librerias necesarias
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoJson.h>

//variables para conectar al wifi
const char* ssid = "********";
const char* password = "*******";

//variables globales para el led
const int ledPin = 2;
const int freq = 5000;
const int ledChannel = 0;
const int resolution = 8;
int ledValue = 0;

//variables globales para potenciometro
const int potPin = 34;
int potValue = 0;
int lastPotValue = -1;

//define un objeto de tipo WebServer que se ejecuta en el puerto 80
AsyncWebServer server(80);
//define un objeto de tipo WebSocket que se ejecuta en el la direccion ws
AsyncWebSocket ws("/ws");

//HTML Y JS
const char html[] PROGMEM = R"rawliteral(
<!DOCTYPE HTML lang>
<html lang="en">
<head>
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>Web Server ESP32</title>
</head>
<body class="" style="background-color: #0c1221;">  
  <section>
  <div class="relative overflow-hidden bg-no-repeat bg-cover bg-center" style="background-image: url('https://i.imgur.com/9WrNbzM.jpg'); height: 300px;"></div>
  <div class="container mx-auto w-3/4 sm:w-3/5 2xl:w-1/2">
      <div class="text-center">
      <div class="block rounded-xl shadow-xl px-6 py-6 lg:py-12 md:px-6" style="margin-top: -100px; background: rgba(3, 23, 42,0.6); backdrop-filter: blur(30px);">
          <h1 class="text-4xl lg:text-5xl font-bold tracking-tight text-teal-500">Web Server ESP32</h1>
          <h2 class="font-bold text-slate-200 mt-3 text-3xl lg:text-4xl">Potentiometer Data Reader and LED Brightness Controller</h2>
      </div>
      </div>
  </div>
  </section>
  <div class="grid grid-cols-1 grid-rows-2 gap-y-12  px-12 my-12 lg:gap-x-12 lg:grid-cols-2">
    <div class="flex flex-col items-center bg-slate-900 shadow-xl rounded-xl shadow-xl py-12 row-span-2" style=" background: rgba(3, 23, 42,0.6); backdrop-filter: blur(30px);">
      <h3 class="text-teal-500 font-bold text-3xl text-center">LED Controller</h3>
      <div class="mt-8 flex items-center">
        <span class="mr-3 text-lg font-bold text-gray-900 dark:text-gray-300">Off</span>
        <label class="relative inline-flex items-center cursor-pointer">
          <input type="checkbox" id="checkbox" value="" class="sr-only peer">
          <div class="w-20 h-10 bg-gray-200 peer-focus:outline-none peer-focus:ring-4
          peer-focus:ring-blue-300 dark:peer-focus:ring-blue-800 rounded-full peer dark:bg-gray-700 peer-checked:after:translate-x-full
          peer-checked:after:border-white after:content-[''] after:absolute after:top-0.5 after:left-[4px] after:bg-white after:border-gray-300
           after:border after:rounded-full after:h-9 after:w-9 after:transition-all dark:border-gray-600 peer-checked:bg-blue-600"></div>
        </label>
        <span class="ml-3 text-lg font-bold text-gray-900 dark:text-gray-300">On</span>
      </div>
      <div class="flex flex-col mt-10 items-center w-3/4 sm:w-96">
        <label class="text-slate-100 font-bold mt-2 text-lg sm:text-xl text-center
        text-center">Brightness: <span class="span" id="brightnessValue">%VALUE%</span></label>
        <input type="range" id="slider" min="0" max="255" step="1" class="w-full h-4 bg-slate-300 rounded-full overflow-hidden mt-3 cursor-pointer">
      </div>
      <div class="flex flex-col items-center mt-8 w-3/4 sm:w-96">
        <span class="text-slate-100 font-bold text-lg text-center sm:text-xl">Type the value brightness</span>
        <div class="flex mt-4 w-full">
          <input id="input" type="number" value="0" class="h-9 w-3/5 sm:w-5/6 bg-slate-200 rounded-l-xl pl-4 text-lg font-bold text-slate-900 outline-none"  min="0" max="255">
          <button id="button" class="h-9 w-2/5 sm:w-1/6  bg-blue-600 rounded-r-xl text-white font-bold px-4 text-lg">Send</button>
        </div>
      </div>
    </div>
    <div class="flex flex-col items-center bg-slate-900 shadow-xl rounded-xl shadow-xl py-12 row-span-2" style=" background: rgba(3, 23, 42,0.6); backdrop-filter: blur(30px);">
      <div class="flex flex-col items-center w-full">
        <h3 class="text-teal-500 font-bold text-3xl text-center">Potentiometer</h3>
        <p class="text-slate-100 mt-8 font-bold text-lg sm:text-xl text-center">Color controlled with potentiometer</p>
        <button id="button-pot" class="w-3/5 sm:w-80 h-36 mt-4 rounde rounded-xl" style="background-color: rgb(255, 255, 255);"></button>
      </div>
    </div>
  </div>
  <footer class="flex flex-col w-full items-center shadow-xl"  style=" background: rgba(3, 23, 42,0.6); backdrop-filter: blur(30px);">
      <img class="w-10 h-auto mt-4" src="https://i.imgur.com/9gTfsBV.png" alt="">
      <p class="text-slate-100 text-center mt-1 mb-3 font-bold">Sistemas Embebidos - 2023</p>
  </footer>
  <script>
    const checkbox = document.getElementById("checkbox");
    const slider = document.getElementById('slider');
    const span = document.getElementById("brightnessValue");
    const button = document.getElementById("button");
    const input = document.getElementById("input");
    const buttonPot = document.getElementById("button-pot");

    const gateway = `ws://${window.location.hostname}:${window.location.port}/ws`;
    let websocket;
    
    window.addEventListener('load', onLoad);
    
    function initWebSocket() {
      console.log('Trying to open a WebSocket connection...');
      websocket = new WebSocket(gateway);
      websocket.onopen    = onOpen;
      websocket.onclose   = onClose;
      websocket.onmessage = onMessage;
    }
    function onOpen(event) {
      console.log('Connection opened');
    }
    function onClose(event) {
      console.log('Connection closed');
      setTimeout(initWebSocket, 2000);
    }
    function onMessage(event) {
      const json = JSON.parse(event.data);
      if ('ledValue' in json) {
        updateledUI(json.ledValue);
      } else {
        updatePotUI(json.potValue);
      }
    }
    function onLoad(event) {
      initWebSocket();
      initDOMFunctions();
    }

    function initDOMFunctions() {
      updateledUI(parseInt(span.innerText));

      checkbox.onchange = () => {
        const isChecked = checkbox.checked;
        const ledValue = isChecked ? '255' : '0';
        updateledUI(ledValue);
        websocket.send(ledValue);
      }

      slider.oninput = () => {
        const ledValue = slider.value;
        updateledUI(ledValue);
        websocket.send(ledValue);
      }

      button.onclick = () => {
        const inputVal = input.value;
        if (isValidNumber(inputVal)) {
          const ledValue = inputVal;
          slider.value = ledValue;
          updateledUI(ledValue);
          websocket.send(ledValue);
        } else {
          alert('NOT A VALID NUMBER!!');
        }
      }

    }

    function updateledUI(value) {
      slider.value = value;
      span.innerHTML = value;
      input.value = value;
      checkbox.checked = value !== 0;
    }

    function updatePotUI(value) {
      const color = `rgb(${255 - value * 2.55}, ${255 - value * 2.55}, 255)`;
      buttonPot.style.backgroundColor = color;
    }

    function isValidNumber(val) {
      if (isNaN(val) || val.trim() === '') {
        return false;
      }
      
      val = parseInt(val);

      if (val >= 0 && val <= 255) {
        return true;
      } else {
        return false;
      }
    }

  </script>
  <script src="https://cdn.tailwindcss.com"></script>
</body>
</html>
)rawliteral";