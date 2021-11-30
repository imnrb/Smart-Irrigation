//Sending Data from Arduino to Esp8266 (ESP-01) Via Serial Communication
//DHT11 Lib
#include <DHT.h>

//Arduino to Esp8266 Lib
#include <ArduinoJson.h>

//Initialise Arduino to Esp8266 (PA10=Rx & PA9=Tx)
HardwareSerial Serial1(PA10,PA9);

//Initialisation of DHT11 Sensor
#define DHTPIN PA0
DHT dht(DHTPIN, DHT11);
int sensorPin = PA1;
int Esp_Input = PA4;
int MotorPin = PB0; 
float sensorValue; 
int limit = 30; // in %
float Moisture;
float temp;
float hum; 
String motorStatus;

void setup() {
  Serial.begin(9600); 
  Serial1.begin(9600);
  dht.begin();
  pinMode(Esp_Input,INPUT);
  pinMode(MotorPin,OUTPUT);
  delay(1000);

  Serial.println("Program started");
}

void loop() {

  StaticJsonBuffer<100> jsonBuffer;
  JsonObject& data = jsonBuffer.createObject();

  //Obtain Moisture Temp and Hum data
  sensor_func();


  //Assign collected data to JSON Object
  data["humidity"] = hum;
  data["temperature"] = temp; 
  data["MoistureValue"] = Moisture; 
  data["MotorStatus"] = motorStatus;

  //Send data to Esp8266
  Serial.println("Data Sent");
  data.printTo(Serial1);
  jsonBuffer.clear();
  
  int on = digitalRead(Esp_Input);
  if(on == 1 || Moisture <= limit){
    digitalWrite(MotorPin,LOW);
    motorStatus = "On";
    Serial.println("Pump ON");
  }
  else{
    digitalWrite(MotorPin,HIGH);
    motorStatus = "Off";
    Serial.println("Pump OFF");
  }
  delay(2000);
  } 

void sensor_func() {

  hum = dht.readHumidity();
  temp = dht.readTemperature();
  sensorValue = analogRead(sensorPin);
  Moisture = 100-((sensorValue/1024)*100);
  Serial.print("Humidity: ");
  Serial.println(hum);
  Serial.print("Temperature: ");
  Serial.println(temp);
  Serial.print("Moisture: ");
  Serial.println(Moisture);

}