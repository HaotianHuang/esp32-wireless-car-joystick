var targetUrl = `ws://${window.location.hostname}/ws`;
var websocket;
window.addEventListener('load', onLoad);


// Adding event listeners to document

var canvas, ctx;


function onLoad() {
  initializeSocket();

  canvas = document.getElementById('canvas');
  ctx = canvas.getContext('2d');          
  resize(); 

  document.addEventListener('mousedown', startDrawing);
  document.addEventListener('mouseup', stopDrawing);
  document.addEventListener('mousemove', Draw);

  document.addEventListener('touchstart', startDrawing);
  document.addEventListener('touchend', stopDrawing);
  document.addEventListener('touchcancel', stopDrawing);
  document.addEventListener('touchmove', Draw);
  window.addEventListener('resize', resize);

  document.getElementById("x_coordinate").innerText = 0;
  document.getElementById("y_coordinate").innerText = 0;
  document.getElementById("speed").innerText = 0;
  document.getElementById("angle").innerText = 0;
}

function initializeSocket() {
  console.log('Opening WebSocket connection to ESP32...');
  websocket = new WebSocket(targetUrl);
  websocket.onopen = onOpen;
  websocket.onclose = onClose; 
  websocket.onmessage = onMessage;
}
function onOpen(event) {
  console.log('Starting connection to server..');
}
function onClose(event) {
  console.log('Closing connection to server..');
  setTimeout(initializeSocket, 2000);
}
function onMessage(event) {
  console.log("WebSocket message received:", event)
}

function send(x,y){
  // var data = {"x":x,"y":y,"speed":speed,"angle":angle};
  // data = JSON.stringify(data);
  const data = [x, y];
  console.log(data);
  websocket.send(data);
}


// Drawing the joystick

var width, height, radius, x_orig, y_orig;
function resize() {
width = window.innerWidth;
radius = 200;
height = radius * 6.5;
ctx.canvas.width = width;
ctx.canvas.height = height;
background();
joystick(width / 2, height / 3);
}

function background() {
x_orig = width / 2;
y_orig = height / 3;

ctx.beginPath();
ctx.arc(x_orig, y_orig, radius + 20, 0, Math.PI * 2, true);
ctx.fillStyle = '#ECE5E5';
ctx.fill();
}

function joystick(width, height) {
ctx.beginPath();
ctx.arc(width, height, radius, 0, Math.PI * 2, true);
ctx.fillStyle = '#F08080';
ctx.fill();
ctx.strokeStyle = '#F6ABAB';
ctx.lineWidth = 8;
ctx.stroke();
}

let coord = { x: 0, y: 0 };
let paint = false;

function getPosition(event) {
var mouse_x = event.clientX || event.touches[0].clientX;
var mouse_y = event.clientY || event.touches[0].clientY;
coord.x = mouse_x - canvas.offsetLeft;
coord.y = mouse_y - canvas.offsetTop;
}

function is_it_in_the_circle() {
var current_radius = Math.sqrt(Math.pow(coord.x - x_orig, 2) + Math.pow(coord.y - y_orig, 2));
if (radius >= current_radius) return true
else return false
}


function startDrawing(event) {
paint = true;
getPosition(event);
if (is_it_in_the_circle()) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    background();
    joystick(coord.x, coord.y);
    Draw();
}
}


function stopDrawing() {
paint = false;
ctx.clearRect(0, 0, canvas.width, canvas.height);
background();
joystick(width / 2, height / 3);
document.getElementById("x_coordinate").innerText = 0;
document.getElementById("y_coordinate").innerText = 0;
document.getElementById("speed").innerText = 0;
document.getElementById("angle").innerText = 0;

}

// Sending and drawing

// Prevent scrolling on every click!
// super sweet vanilla JS delegated event handling!
document.body.addEventListener("click", function (e) {
  if (e.target && e.target.nodeName == "A") {
    e.preventDefault();
  }
});


function Draw(event) {

if (paint) {
    ctx.clearRect(0, 0, canvas.width, canvas.height);
    background();
    var angle_in_degrees,x, y, speed;
    var angle = Math.atan2((coord.y - y_orig), (coord.x - x_orig));

    if (Math.sign(angle) == -1) {
        angle_in_degrees = Math.round(-angle * 180 / Math.PI);
    }
    else {
        angle_in_degrees =Math.round( 360 - angle * 180 / Math.PI);
    }


    if (is_it_in_the_circle()) {
        joystick(coord.x, coord.y);
        x = coord.x;
        y = coord.y;
    }
    else {
        x = radius * Math.cos(angle) + x_orig;
        y = radius * Math.sin(angle) + y_orig;
        joystick(x, y);
    }


    getPosition(event);

    var speed =  Math.round(100 * Math.sqrt(Math.pow(x - x_orig, 2) + Math.pow(y - y_orig, 2)) / radius);

    var x_relative = Math.round(x - x_orig);
    var y_relative = Math.round(y - y_orig);
    

    document.getElementById("x_coordinate").innerText =  x_relative;
    document.getElementById("y_coordinate").innerText = -y_relative ;
    document.getElementById("speed").innerText = speed;
    document.getElementById("angle").innerText = angle_in_degrees;

    send(x_relative,-y_relative);//,speed,angle_in_degrees);
}
} 
    

// /*
// Speed Settings Handler
// */
// var speedSettings = document.querySelectorAll(
//   'input[type=radio][name="speed-settings"]'
// );
// speedSettings.forEach((radio) =>
//   radio.addEventListener("change", () => 
//   {
//     var speedSettings = radio.value;
//     console.log('Speed Settings :: ' + speedSettings)
//     sendMessage(speedSettings); 
//   }
// ));







