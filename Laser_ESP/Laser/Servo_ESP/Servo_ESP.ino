#include <Servo.h>
#include <WiFi.h>
#include <ESPmDNS.h>
#include <WiFiClient.h>
#include <ArduinoJson.h>

// create servo object to control a servo
Servo servo1;
Servo servo2;

// GPIO the servo is attached to
static const int servoPin1 = 13;
static const int servoPin2 = 12;


int angle1 = 0;
int angle2 = 0;

//Credentials to use wifi
const char* ssid     = "INFINITUM2711";
const char* password = "5KOEcUB87j";

// Set web server port number to 80
WiFiServer server(80);

bool endMessage = false;

void setup() {
  Serial.begin(115200);

// attaches the servo on the servoPin
  servo1.attach(servoPin1);
  servo2.attach(servoPin2);

  wifiConnect();

}

void loop() {
  
WiFiClient sclient = server.available();   // Listen for incoming


 if (sclient) {                             // If a new client connects,
    Serial.println("New Client.");         // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (sclient.connected()) {           // loop while the client's connected

    if (sclient.available()){              // if there's bytes to read from the client,
        char c = sclient.read();            // read a byte, then
       
        Serial.write(c);                   // print it out the serial monitor
    
         if(c == '}'){ endMessage = true; }
         if (c == '\n') {                   // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {

           
          } else {    // if you got a newline, then clear currentLine:
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }

        if(endMessage){
          String JSONMenssage = currentLine;
            
            StaticJsonBuffer<100> JSONBuffer;
            JsonObject& objectJSON = JSONBuffer.parseObject(JSONMenssage);
            if(objectJSON.success()){
              angle1 = objectJSON ["angle1"];
              angle2 = objectJSON ["angle2"];
              servo1.write(angle1);
              servo2.write(angle2);
              sclient.println("HTTP/1.1 200 OK");
              sclient.println("Connection: close");
              sclient.stop();
             
        }

                   
      }
      
    }
       
  }
   
  Serial.println("Client disconnected. \n");
}
}

void wifiConnect(){

// Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
// Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());

  if (!MDNS.begin("robot")) {
      
    }
   else{
   server.begin();
   MDNS.addService("http", "tcp", 80); 
   }
  
  
}
