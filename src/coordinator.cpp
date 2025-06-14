#include <HardwareSerial.h>         // Include HardwareSerial library for communication
#include <Arduino.h> 

HardwareSerial XBee(2);             // Use Serial2 (UART2)
 
bool started = false;               // True when start marker is detected
bool ended = false;                 // True when end marker is detected
char incomingByte;                  // Storage for each byte read
char msg[8];                        // Increased buffer size for larger numbers
int bufferPos = 0;                  // Current position in the array
 
void setup() {
  Serial.begin(115200);               // Start communication with the PC for debugging
  XBee.begin(9600, SERIAL_8N1, 25, 26);  // Configure Serial2: RX=25, TX=26
  
  Serial.println("Coordinator started, waiting for data...");
  delay(1000);
}
 
void loop() {
  delay(5000);
  if (!XBee.available()){
    Serial.println("No data available from XBee.");
  }
  // Add debugging to see if any data is coming in
  else if (XBee.available() > 0) {
    Serial.println("Data available from XBee!");
  
  
    while (XBee.available() > 0) {    // Check if there is data available from the XBee
      incomingByte = XBee.read();     // Read the incoming byte
      
      // Debug: print every byte received
      Serial.print("Received byte: ");
      Serial.print(incomingByte);
      Serial.print(" (");
      Serial.print((int)incomingByte);
      Serial.println(")");
  
      if (incomingByte == '<') {      // Detect start of the message
        started = true;
        bufferPos = 0;                // Reset buffer position
        msg[bufferPos] = '\0';        // Clear the buffer
        Serial.println("Start marker detected");
      }
      else if (incomingByte == '>') { // Detect end of the message
        ended = true;
        Serial.println("End marker detected");
        break;                        // Stop reading, process the message
      }
      else if (started && bufferPos < 7) {    // Store the byte in msg array if message has started
        msg[bufferPos] = incomingByte;
        bufferPos++;                  // Increment buffer position
        msg[bufferPos] = '\0';        // Null terminate the string
      }
    }
  
    if (started && ended) {                // If a complete message was received, process it
      int value = atoi(msg);               // Convert the buffer to an integer
      Serial.print("Complete message received: '");
      Serial.print(msg);
      Serial.print("' = ");
      Serial.println(value);
  
      started = false;                     // Reset for the next message
      ended = false;
      bufferPos = 0;                       // Reset buffer position for next message
    }
    
    delay(100);  // Small delay to prevent overwhelming the serial monitor
  }
}