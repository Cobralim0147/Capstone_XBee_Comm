#include <HardwareSerial.h>         
#include <Arduino.h> 

HardwareSerial XBee(2);             
 
bool started = false;               
bool ended = false;                 
char incomingByte;                  
char msg[8];                        
int bufferPos = 0;                  
 
void setup() {
  Serial.begin(115200);               
  delay(3000);  // Wait for Serial Monitor
  
  Serial.println("=============================");
  Serial.println("COORDINATOR STARTING...");
  Serial.println("=============================");
  
  XBee.begin(9600, SERIAL_8N1, 25, 26);  
  delay(1000);
  
  Serial.println("XBee initialized on pins 25(RX), 26(TX)");
  Serial.println("Coordinator ready, waiting for data...");
  Serial.println("=============================");
}
 
void loop() {
  static unsigned long lastCheck = 0;
  static unsigned long messageCount = 0;
  
  // Print status every 5 seconds
  if (millis() - lastCheck > 5000) {
    Serial.print("Status: Alive | Messages received: ");
    Serial.print(messageCount);
    Serial.print(" | XBee available: ");
    Serial.println(XBee.available());
    lastCheck = millis();
  }
  
  if (XBee.available() > 0) {
    Serial.println(">>> DATA AVAILABLE FROM XBEE! <<<");
    
    while (XBee.available() > 0) {    
      incomingByte = XBee.read();     
      
      Serial.print("Byte: '");
      Serial.print(incomingByte);
      Serial.print("' (ASCII: ");
      Serial.print((int)incomingByte);
      Serial.println(")");
  
      if (incomingByte == '<') {      
        started = true;
        bufferPos = 0;                
        msg[bufferPos] = '\0';        
        Serial.println(">>> START MARKER DETECTED <<<");
      }
      else if (incomingByte == '>') { 
        ended = true;
        Serial.println(">>> END MARKER DETECTED <<<");
        break;                        
      }
      else if (started && bufferPos < 7) {    
        msg[bufferPos] = incomingByte;
        bufferPos++;                  
        msg[bufferPos] = '\0';        
      }
    }
  
    if (started && ended) {                
      int value = atoi(msg);               
      messageCount++;
      Serial.println("=============================");
      Serial.print("COMPLETE MESSAGE #");
      Serial.print(messageCount);
      Serial.print(": '");
      Serial.print(msg);
      Serial.print("' = ");
      Serial.println(value);
      Serial.println("=============================");
  
      started = false;                     
      ended = false;
      bufferPos = 0;                       
    }
  }
  
  delay(100);  
}

