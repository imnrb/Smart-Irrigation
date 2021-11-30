// Load Wi-Fi library
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid     = "Roland";
const char* password = "12345678";

SoftwareSerial esp01(0,1);  //Rx=0,Tx=1  (GPIO0 and GPIO1)

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

// Auxiliar variables to store the current output state
String output2State = "off";


// Assign output variables to GPIO pins
const int output2 = 2; //GPIO2


// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  esp01.begin(9600);
  //while (!Serial) continue;
  Serial.begin(115200);
  // Initialize the output variables as outputs
  pinMode(output2, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output2, LOW);

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
  server.begin();
}

void loop(){
  WiFiClient client = server.available();   // Listen for incoming clients
  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& data = jsonBuffer.parseObject(esp01);

  //if (data == JsonObject::invalid()) {
    //Serial.println("Invalid Json Object");
    //jsonBuffer.clear();
    //return;
  //}

  Serial.println("JSON Object Recieved");
  Serial.print("Recieved Moisture Sensor Value:  ");
  float sensValue = data["MoistureValue"];
  Serial.println(sensValue);
  Serial.print("Recieved Humidity:  ");
  float hum =data["humidity"];
  Serial.println(hum);
  Serial.print("Recieved Temperature:  ");
  float temp = data["temperature"];
  Serial.println(temp);
  String motorStatus = data["MotorStatus"];
  Serial.println(motorStatus);
  Serial.println("-----------------------------------------");
   

   
  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    currentTime = millis();
    previousTime = currentTime;
    while (client.connected() && currentTime - previousTime <= timeoutTime) { // loop while the client's connected
      currentTime = millis();         
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /2/on") >= 0) {
              Serial.println("GPIO 2 on");
              output2State = "on";
              digitalWrite(output2, HIGH);
            } else if (header.indexOf("GET /2/off") >= 0) {
              Serial.println("GPIO 2 off");
              output2State = "off";
              digitalWrite(output2, LOW);
            } 
            
            // Display the HTML web page
    client.println("<!DOCTYPE html><html>");
    
    client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
    client.println("<link rel=\"icon\" href=\"data:,\">");
    // CSS to style the on/off buttons 
    // Feel free to change the background-color and font-size attributes to fit your preferences
    client.println("<style>html { background-color:#c2fff6;font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
    client.println(".button { display: inline-block;padding: .75rem 1.25rem;border-radius: 10rem;color: #fff; background-color: #33de4a;text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");   //#403737,#e61515,EEFBFB,#33de4a=Green
   
    client.println(".button1 {background-color: #33de4a;}");
    client.println(".button2 {background-color: #e61515;}");
    client.println(".h1{font-family: times, Times New Roman, times-roman, georgia, serif;color: #444;margin: 0;padding: 0px 0px 6px 0px;font-size: 51px;line-height: 44px;letter-spacing: -2px;font-weight: bold;}");
    client.println(".h2{font-family: Georgia, Times New Roman, Times, serif;font-size:24px;margin-top: 5px; margin-bottom: 0px;text-align: center;font-weight: normal;color: #222;}");
    client.println(".fas{color:#36d457;animation: blink 2s linear infinite alternate;position: relative;left: 30px;top:20px;}");
    client.println("@keyframes blink {from {transform: scale(1);}to {transform: scale(1.5)}}</style>");
    client.println("<script src='https://kit.fontawesome.com/054d978d26.js' crossorigin='anonymous'>function myFunction() {alert('The Pump is Turned On');</script></head>");
    
    // Web Page Heading
    client.println("<body><h1>Water Your Plants Here </h1>");
    
    // Display current state, and ON/OFF buttons for GPIO 2  
    // client.println("<p>GPIO 2 - State " + output2State + "</p>");
    // If the output2State is off, it displays the ON button       
    if (output2State=="off") {
      client.println("<p><a href=\"/2/on\"><button class=\"button button1\">Switch Pump On</button></a></p>");
    } else {
      client.println("<p><a href=\"/2/off\"><button class=\"button button2\">Switch Pump Off</button></a></p>");
    } 
    client.print("<body><h2>Pump(Motor) Status: ");
    client.print(motorStatus);
    client.print("</h2>");
    client.println();
    client.println("<body><h2>The Moisture Sensor Value : </h2>");
    client.print("<h2>");
    client.print(sensValue);
    client.print(" %");
    client.print("</h2>");
    client.println("<body><h2>The Humidity of surrounding is </h1>");
    client.print("<h2>");
    client.print(hum);
    client.print(" %");
    client.print("</h2>");
    client.println();
    client.println("<body><h2>The Temperature of surrounding is </h1>");
    client.print("<h2>");
    client.print(temp);
    client.print(" &#8451");
    client.print("</h2>");
    if (temp >= 30)
    {
      client.println("<h2 style='color:red;'>The Temperature is High.</h2>");
    }
    if (hum < 30)
    {
      client.println("<h2 style='color:red;'>The Humidity is Low.</h2>");
    }if (hum > 60)
    {
      client.println("<h2 style='color:red;'>The Humidity is High.</h2>");
    }
    client.println("<br>");
    client.print("<i class='fas fa-seedling fa-5x'></i>");
    
    client.println("</body></html>");
    
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}