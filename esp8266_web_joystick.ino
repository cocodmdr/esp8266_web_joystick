#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266WiFi.h>

//Set Wifi ssid and password
const char WiFiSSID[] = "wifiRobot";
const char WiFiAPPSK[] = "joystick";

const int joystickRadius = 200;
const int maxSpeed = 1023;
const float turningCoefficient = 0.5;
  
struct robotSpeed {
    bool robotDirection;
    int leftWheelSpeed;
    int rightWheelSpeed;
};

ESP8266WebServer server (80);

//This function takes the parameters passed in the URL(the x and y coordinates of the joystick)
//and sets the motor speed based on those parameters. 
void handleJSData(){

  int x = server.arg(0).toInt();
  int y = server.arg(1).toInt();

  struct robotSpeed myRobotSpeed;
  myRobotSpeed = calculateSpeeds(x,y);
  
  move(myRobotSpeed.robotDirection, myRobotSpeed.leftWheelSpeed, myRobotSpeed.rightWheelSpeed);

  //return an HTTP 200
  server.send(200, "text/plain", "");   
}

struct robotSpeed calculateSpeeds(int x, int y){
  struct robotSpeed myRobotSpeed;
  
  myRobotSpeed.robotDirection = y > 0;

  int normalizedX = map(x, -joystickRadius, joystickRadius, -maxSpeed, maxSpeed);
  int normalizedY = map(abs(y), 0, joystickRadius, 0.0, maxSpeed);
  
  int leftWheelSpeed = normalizedY + normalizedX * turningCoefficient;
  int rightWheelSpeed = normalizedY - normalizedX * turningCoefficient;

  myRobotSpeed.leftWheelSpeed = constrain(leftWheelSpeed, 0, maxSpeed);
  myRobotSpeed.rightWheelSpeed = constrain(rightWheelSpeed, 0, maxSpeed);
  
  return myRobotSpeed;
}

void move(bool direction, int leftWheelSpeed, int rightWheelSpeed) {
  Serial.print(" L ");
  Serial.print(leftWheelSpeed);
  Serial.print(" R ");
  Serial.println(rightWheelSpeed);
  
  if(direction){
    analogWrite(D1, rightWheelSpeed);
    analogWrite(D2, 0);
    analogWrite(D3, leftWheelSpeed);
    analogWrite(D4, 0);
  } else {
    analogWrite(D1, 0);
    analogWrite(D2, rightWheelSpeed);
    analogWrite(D3, 0);
    analogWrite(D4, leftWheelSpeed);
  }
}

void setupBuggy()
{
  pinMode(D1, OUTPUT);
  pinMode(D2, OUTPUT);
  pinMode(D3, OUTPUT);
  pinMode(D4, OUTPUT);
  
  stopBuggy();
}

void stopBuggy()
{
  analogWrite(D1, 0);
  analogWrite(D2, 0);
  analogWrite(D3, 0);
  analogWrite(D4, 0);
}

void setup()
{
  setupBuggy();
  
  // Debug console
  Serial.begin(115200);
  Serial.setDebugOutput(true);
  
  WiFi.mode(WIFI_AP);
  WiFi.softAP(WiFiSSID, WiFiAPPSK);

  //initialize SPIFFS to be able to serve up the static HTML files. 
  if (!SPIFFS.begin()){
    Serial.println("SPIFFS Mount failed");
  } 
  else {
    Serial.println("SPIFFS Mount succesfull");
  }
  //set the static pages on SPIFFS for the html and js
  server.serveStatic("/", SPIFFS, "/joystick.html"); 
  server.serveStatic("/virtualjoystick.js", SPIFFS, "/virtualjoystick.js");
  //call handleJSData function when this URL is accessed by the js in the html file
  server.on("/jsData.html", handleJSData);  
  server.begin();
}

void loop()
{
  server.handleClient();  
}
