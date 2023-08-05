#include <ESP8266WebServer.h>
#include <FS.h>
#include <ESP8266WiFi.h>

//Set Wifi ssid and password
const char WiFiSSID[] = "collector";
const char WiFiAPPSK[] = "collector";

ESP8266WebServer server (80);

//This function takes the parameters passed in the URL(the x and y coordinates of the joystick)
//and sets the motor speed based on those parameters. 
void handleJSData(){
  boolean yDir;
  int x = server.arg(0).toInt() * 10;
  int y = server.arg(1).toInt() * 10;
  int aSpeed = abs(y);
  int bSpeed = abs(y);
  //set the direction based on y being negative or positive
  if ( y < 0 ){
    yDir = 0; 
  }
  else { 
    yDir = 1;
  }  
  //adjust to speed of each each motor depending on the x-axis
  //it slows down one motor and speeds up the other proportionately 
  //based on the amount of turning
  aSpeed = constrain(aSpeed + x/2, 0, 1023);
  bSpeed = constrain(bSpeed - x/2, 0, 1023);

  //use the speed and direction values to turn the motors
  //if either motor is going in reverse from what is expected,
  //just change the 2 digitalWrite lines for both motors:
  //!ydir would become ydir, and ydir would become !ydir
  
  // Move
  if(!yDir){
    analogWrite(D1, 0);
    analogWrite(D2, bSpeed);
    analogWrite(D3, 0);
    analogWrite(D4, aSpeed);
  } else {
    analogWrite(D1, bSpeed);
    analogWrite(D2, 0);
    analogWrite(D3, aSpeed);
    analogWrite(D4, 0);
  }

  //return an HTTP 200
  server.send(200, "text/plain", "");   
}

void setupBuggy()
{
  // We will use these 4 output PWMs
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
