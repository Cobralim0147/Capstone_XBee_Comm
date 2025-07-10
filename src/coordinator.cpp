// Library
#include <HardwareSerial.h>         
#include <Arduino.h> 

// Pin layout          
int LED_PIN = 33;

// Global variables
float temperature = 0.0;
float humidity = 0.0;
int receivedNumber = 0;
bool tempFound = false;
bool humFound = false;
bool numberFound = false;
bool started = false;
bool ended = false;
String message = "";

// Initialization
HardwareSerial XBee(2);   

void blinkLED() {
  digitalWrite(LED_PIN, HIGH);  
  delay(1000);
  digitalWrite(LED_PIN, LOW);   
  delay(1000);
}

void setup() {
  Serial.begin(115200);               
  delay(3000);  
  
  Serial.println("=============================");
  Serial.println("COORDINATOR STARTING...");
  Serial.println("=============================");
  
  // Pin setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  

  XBee.begin(9600, SERIAL_8N1, 25, 26);  
  delay(1000);
  blinkLED();
}

void loop() {
  while (XBee.available() > 0) {
    Serial.println("Data available from XBee");
    char incomingByte = XBee.read();

    if (incomingByte == '<') {
      started = true;
      ended = false;
      message = "";
    }
    else if (incomingByte == '>') {
      ended = true;
      break;  
    }
    else if (started) {
      message += incomingByte;  
    }
  }
  
  if (started && ended) {
    // Complete message received
    blinkLED();
    
    Serial.println("=============================");
    Serial.print("Raw message: ");
    Serial.println(message);
    
    // Find number
    int numIndex = message.indexOf("Number: ");
    if (numIndex != -1) {
      int numStart = numIndex + 8;
      int numEnd = message.indexOf(",", numStart);
      if (numEnd == -1) numEnd = message.indexOf(" ", numStart);
      if (numEnd == -1) numEnd = message.length();
      
      if (numEnd != -1) {
        receivedNumber = message.substring(numStart, numEnd).toInt();
        numberFound = true;
      }
    }
    
    // Find temperature
    int tempIndex = message.indexOf("Temperature: ");
    if (tempIndex != -1) {
      int tempStart = tempIndex + 13;
      int tempEnd = message.indexOf(" °C", tempStart);
      if (tempEnd == -1) tempEnd = message.indexOf(",", tempStart);
      if (tempEnd == -1) tempEnd = message.length();
      
      if (tempEnd != -1) {
        temperature = message.substring(tempStart, tempEnd).toFloat();
        tempFound = true;
      }
    }
    
    // Find humidity
    int humIndex = message.indexOf("Humidity: ");
    if (humIndex != -1) {
      int humStart = humIndex + 10;
      int humEnd = message.indexOf(" %", humStart);
      if (humEnd == -1) humEnd = message.indexOf(",", humStart);
      if (humEnd == -1) humEnd = message.length();
      
      if (humEnd != -1) {
        humidity = message.substring(humStart, humEnd).toFloat();
        humFound = true;
      }
    }
    
    // Display results
    if (numberFound) {
      Serial.print("Received Number: ");
      Serial.println(receivedNumber);
    }
    if (tempFound) {
      Serial.print("Temperature: ");
      Serial.print(temperature, 2);
      Serial.println(" °C");
    }
    if (humFound) {
      Serial.print("Humidity: ");
      Serial.print(humidity, 2);
      Serial.println(" %");
    }
    
    if (!tempFound && !humFound && !numberFound) {
      Serial.println("Error: Could not parse any data from message");
    }
    
    Serial.println("=============================");
    
    // Reset for next message
    started = false;
    ended = false;
    message = "";
  }
}