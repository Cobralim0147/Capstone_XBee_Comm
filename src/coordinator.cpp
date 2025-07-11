// Library
#include <HardwareSerial.h>         
#include <Arduino.h> 

// Pin layout          
int LED_PIN = 33;

// Global variables
float temperature = 0.0;
float humidity = 0.0;
int receivedNumber = 0; 
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
    // Serial.println("Data available from XBee");
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
    
    // Convert String to char array for C-style parsing
    char msg[message.length() + 1];
    message.toCharArray(msg, message.length() + 1);
    
    // Extract number, temperature and humidity using C-style approach
    char* numPtr = strstr(msg, "N:");          // Find "N:" in message
    char* tempPtr = strstr(msg, "T:");         // Find "T:" in message
    char* humPtr = strstr(msg, "H:");          // Find "H:" in message
    
    bool numberFound = false;
    bool tempFound = false;
    bool humFound = false;
    
    if (numPtr) {
      receivedNumber = atoi(numPtr + 2);       
      numberFound = true;
    }
    
    if (tempPtr) {
      temperature = atof(tempPtr + 2);         
      tempFound = true;
    }
    
    if (humPtr) {
      humidity = atof(humPtr + 2);            
      humFound = true;
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
    
    if (!numberFound && !tempFound && !humFound) {
      Serial.println("Error: Could not parse any data from message");
    }
    
    Serial.println("=============================");
    
    // Reset for next message
    started = false;
    ended = false;
    message = "";
  }
}