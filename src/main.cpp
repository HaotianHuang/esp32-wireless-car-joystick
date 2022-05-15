#include <Arduino.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include "SPIFFS.h"

// #define enA 35
// #define in1 32
// #define in2 33
#define enA 21
#define in1 19
#define in2 18
#define enB 27
#define in3 25
#define in4 26
int motorSpeedA = 0;
int motorSpeedB = 0;

// All this code is assuming regular cartesian plane with driving wheels at front
void sendCarCommand(const char *command)
{
  Serial.println("------");
  Serial.println(command);

  String str = command;
  int comma = str.indexOf(','); 
  int xAxis = str.substring(0, comma).toInt();
  int yAxis = str.substring(comma + 1).toInt();

  // for wheels to lead
  yAxis = map(yAxis, -200, 200, 200, -200);
  xAxis = map(xAxis, -200, 200, 200, -200);


  Serial.println("=====-200 to 200:=====-");
  Serial.println(xAxis);
  Serial.println(yAxis);
  
  xAxis = map(xAxis, -200, 200, 0, 1024);
  yAxis = map(yAxis, -200, 200, 0, 1024);

  Serial.println("=====-0 to 1024:=====-");
  Serial.println(xAxis);
  Serial.println(yAxis);

  Serial.println("=====-Speed A & B=====-");
  Serial.println(motorSpeedA);
  Serial.println(motorSpeedB);
  
  // Y-axis used for forward and backward control
  if (yAxis < 400) {
    Serial.println("down");
    Serial.println("------");
    // Set Motor A backward
    digitalWrite(in1, LOW);
    digitalWrite(in2, HIGH);
    // Set Motor B backward
    digitalWrite(in3, LOW);
    digitalWrite(in4, HIGH);
    // Convert the declining Y-axis readings for going backward from 470 to 0 into 0 to 255 value for the PWM signal for increasing the motor speed
    motorSpeedA = map(yAxis, 400, 0, 0, 200);
    motorSpeedB = map(yAxis, 400, 0, 0, 200);
  }
  else if (yAxis > 600) {
    Serial.println("up");
    Serial.println("------");
    // Set Motor A forward
    digitalWrite(in1, HIGH);
    digitalWrite(in2, LOW);
    // Set Motor B forward
    digitalWrite(in3, HIGH);
    digitalWrite(in4, LOW);
    // Convert the increasing Y-axis readings for going forward from 550 to 1023 into 0 to 255 value for the PWM signal for increasing the motor speed
    motorSpeedA = map(yAxis, 600, 1024, 0, 200);
    motorSpeedB = map(yAxis, 600, 1024, 0, 200);
  }
  // If joystick stays in middle the motors are not moving
  else {
    motorSpeedA = 0;
    motorSpeedB = 0;
  }

  // X-axis used for left and right control
  if (xAxis < 400) {
    // Convert the declining X-axis readings from 470 to 0 into increasing 0 to 255 value
    int xMapped = map(xAxis, 400, 0, 0, 200);
    // Move to left - decrease left motor speed, increase right motor speed
    motorSpeedA = motorSpeedA - xMapped;
    motorSpeedB = motorSpeedB + xMapped;
    // Confine the range from 0 to 255
    if (motorSpeedA < 0) {
      motorSpeedA = 0;
    }
    if (motorSpeedB > 200) {
      motorSpeedB = 200;
    }
  }
  if (xAxis > 600) {
    // Convert the increasing X-axis readings from 550 to 1023 into 0 to 255 value
    int xMapped = map(xAxis, 600, 1024, 0, 200);
    // Move right - decrease right motor speed, increase left motor speed
    motorSpeedA = motorSpeedA + xMapped;
    motorSpeedB = motorSpeedB - xMapped;
    // Confine the range from 0 to 255
    if (motorSpeedA > 200) {
      motorSpeedA = 200;
    }
    if (motorSpeedB < 0) {
      motorSpeedB = 0;
    }
  }
  //Prevent buzzing at low speeds (Adjust according to your motors. My motors couldn't start moving if PWM value was below value of 70)
  if (motorSpeedA < 70) {
    motorSpeedA = 0;
  }
  if (motorSpeedB < 70) {
    motorSpeedB = 0;
  }

  analogWrite(enA, motorSpeedA); // Send PWM signal to motor A
  analogWrite(enB, motorSpeedB); // Send PWM signal to motor B

}


// Change this to your network SSID
const char *ssid = "WiFi-94F5";
const char *password = "HHT1357924680";

// AsyncWebserver runs on port 80 and the asyncwebsocket is initialize at this point also
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

// Callback function that receives messages from websocket client
void onWsEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type,
               void *arg, uint8_t *data, size_t len)
{
  switch (type)
  {
    case WS_EVT_CONNECT:
    {
      // Serial.printf("ws[%s][%u] connect\n", server->url(), client->id());
      // client->printf("Hello Client %u :)", client->id());
      // client->ping();
    }

    case WS_EVT_DISCONNECT:
    {
      // Serial.printf("ws[%s][%u] disconnect\n", server->url(), client->id());
    }

    case WS_EVT_DATA:
    {
      //data packet
      AwsFrameInfo *info = (AwsFrameInfo *)arg;
      if (info->final && info->index == 0 && info->len == len)
      {
        //the whole message is in a single frame and we got all of it's data
        if (info->opcode == WS_TEXT)
        {
          data[len] = 0;
          char *command = (char *)data;
          sendCarCommand(command);
          //Serial.println(command);

        }
      }
    }

    case WS_EVT_PONG:
    {
      // Serial.printf("ws[%s][%u] pong[%u]: %s\n", server->url(), client->id(), len, (len) ? (char *)data : "");
    }

    case WS_EVT_ERROR:
    {
      // Serial.printf("ws[%s][%u] error(%u): %s\n", server->url(), client->id(), *((uint16_t *)arg), (char *)data);
    }
  }
}

// Function called when resource is not found on the server
void notFound(AsyncWebServerRequest *request)
{
  request->send(404, "text/plain", "Not found");
}

// Setup function
void setup()
{

  pinMode(enA, OUTPUT);
  pinMode(enB, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);

  // Initialize the serial monitor baud rate
  Serial.begin(115200);
  Serial.println("Connecting to ");
  Serial.println(ssid);

  // Connect to your wifi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  if (WiFi.waitForConnectResult() != WL_CONNECTED)
  {
    Serial.printf("WiFi Failed!\n");
    return;
  }

  Serial.print("IP Address: ");
  Serial.println(WiFi.localIP());

  // Initialize SPIFFS
  if (!SPIFFS.begin(true))
  {
    Serial.println("An Error has occurred while mounting SPIFFS");
    return;
  }

  // Add callback function to websocket server
  ws.onEvent(onWsEvent);
  server.addHandler(&ws);

  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
            {
              Serial.println("Requesting index page...");
              request->send(SPIFFS, "/index.html", "text/html");
            });

  // Route to load custom.js file
  server.on("/js/custom.js", HTTP_GET, [](AsyncWebServerRequest *request)
            { request->send(SPIFFS, "/js/custom.js", "text/javascript"); });

  // On Not Found
  server.onNotFound(notFound);

  // Start server
  server.begin();

  
}

void loop()
{
  // No code in here.  Server is running in asynchronous mode

}
