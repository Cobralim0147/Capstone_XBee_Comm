#include <HardwareSerial.h>         
#include <Arduino.h> 

HardwareSerial XBee(2);             
int LED_PIN = 32;
int time_check = 15000;
bool started = false;
bool ended = false;
String message = "";

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
}

void blinkLED() {
  digitalWrite(LED_PIN, HIGH);  
  delay(1000);
  digitalWrite(LED_PIN, LOW);   
  delay(1000);
}

void loop() {
  while (XBee.available() > 0) {
    char incomingByte = XBee.read();
    
    if (incomingByte == '<') {
      started = true;
      ended = false;
      message = "";  // Clear previous message
    }
    else if (incomingByte == '>') {
      ended = true;
      break;  // Complete message received
    }
    else if (started) {
      message += incomingByte;  // Build the message
    }
  }
  
  if (started && ended) {
    // Complete message received
    blinkLED();
    Serial.print("Number received: ");
    Serial.println(message.toInt());  // Convert to number
    
    // Reset for next message
    started = false;
    ended = false;
    message = "";
  }
  // else if (!started) {
  //   Serial.println("No message received");
  // }
  
}