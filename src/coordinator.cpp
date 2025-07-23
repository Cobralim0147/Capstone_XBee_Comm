// Library
#include <HardwareSerial.h>         
#include <WiFiManager.h> 
#include <Arduino.h>
#include <Firebase_ESP_Client.h> 
#include <DHT.h>
#include <addons/TokenHelper.h>
#include <addons/RTDBHelper.h>

//Firebase Connection Details
#define API_KEY ""
#define FIREBASE_PROJECT_ID ""
#define DATABASE_URL ""
#define USER_EMAIL ""
#define USER_PASSWORD ""

// Constants
#define SENSOR_NUMBER 13
#define WIFI_CONNECT_TIMEOUT 30000
#define LED_PIN 33

// Global variables
float temperature = 0.0;
float humidity = 0.0;
int receivedNumber = 0; 
bool started = false;
bool ended = false;
bool numberFound = false;
bool tempFound = false;
bool humFound = false;
String message = "";
unsigned long wifiConnectStart = 0;

// State variables
bool firebaseConnected = false;
String documentPath;
int retryCount = 0;

// Initialization
HardwareSerial XBee(2);   

// Firebase variables
FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

// Function prototypes
void setupWiFi();
void setupFirebase();
void blinkLED();
bool uploadToFirebase(float temp, float humid, int number);
void printDataReceived();

void setup() {
  Serial.begin(115200);               
  delay(3000);  
  
  Serial.println("=============================");
  Serial.println("COORDINATOR STARTING...");
  Serial.println("=============================");
  
  // Pin setup
  pinMode(LED_PIN, OUTPUT);
  digitalWrite(LED_PIN, LOW);  

  //Setup XBee Serial
  XBee.begin(9600, SERIAL_8N1, 25, 26);  

  // Setup WiFi and Firebase
  setupWiFi();
  setupFirebase();
  
  // Create document path
  documentPath = "/plants/" + String(SENSOR_NUMBER);
  Serial.println("Setup complete. Starting monitoring...");

  delay(2000); 
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
    // Serial.print("Raw message: ");
    // Serial.println(message);
    
    // Convert String to char array for C-style parsing
    char msg[message.length() + 1];
    message.toCharArray(msg, message.length() + 1);
    
    // Extract number, temperature and humidity using C-style approach
    char* numPtr = strstr(msg, "N:");          // Find "N:" in message
    char* tempPtr = strstr(msg, "T:");         // Find "T:" in message
    char* humPtr = strstr(msg, "H:");          // Find "H:" in message
    
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

    //upload data to Firebase
    if (firebaseConnected) {
      if (uploadToFirebase(temperature, humidity, receivedNumber)) {}
    } else {
      Serial.println("Firebase not connected. Skipping upload.");
    }

    // Display results
    printDataReceived();
    
    Serial.println("=============================");
    
    // Reset for next message
    started = false;
    ended = false;
    message = "";
  }
}

// Print data received
void printDataReceived() {
  if (!numberFound && !tempFound && !humFound) {
    Serial.println("No valid data received.");
    return;
  }
  else{
    Serial.print("Received Number: ");
    Serial.println(receivedNumber);
    Serial.print("Temperature: ");
    Serial.print(temperature, 2);
    Serial.println(" °C");
    Serial.print("Humidity: ");
    Serial.print(humidity, 2);
    Serial.println(" %");
  }
}

// LED blink function
void blinkLED() {
  digitalWrite(LED_PIN, HIGH);  
  delay(1000);
  digitalWrite(LED_PIN, LOW);   
  delay(1000);
}

// Callback for Firebase token status
bool uploadToFirebase(float temp, float humid, int receivedNumber) {
    if (isnan(temp) || isnan(humid) || isnan(receivedNumber)) {
        Serial.println("Failed to read sensor data.");
        return false;
    }
    
    String documentPath = String("/plants/SE_") + String(SENSOR_NUMBER);
    FirebaseJson content;
    
    // Set fields (equivalent format to your original)
    content.set("fields/temperature/doubleValue", String(temp, 2));
    content.set("fields/humidity/doubleValue", String(humid, 2));
    content.set("fields/number/doubleValue", String(receivedNumber));
   
    
    Serial.println("Uploading to Firebase...");
    
    // Single efficient call instead of 4 separate calls
    if (Firebase.Firestore.patchDocument(&fbdo, FIREBASE_PROJECT_ID, "", 
                                        documentPath.c_str(), content.raw(), 
                                        "temperature,humidity,number")) {
        Serial.println("Firebase upload successful");
        return true;
    } else {
        Serial.print("Firebase upload failed: ");
        Serial.println(fbdo.errorReason());
        return false;
    }
}

// setup WiFi
void setupWiFi() {
    WiFi.mode(WIFI_STA);
    WiFiManager wm;
    
    // Set timeout for connection attempts
    wm.setConnectTimeout(WIFI_CONNECT_TIMEOUT / 1000);
    
    Serial.println("Connecting to WiFi...");
    wifiConnectStart = millis();
    
    while (WiFi.status() != WL_CONNECTED) {
        blinkLED(); 

        String apName = "SE_" + String(SENSOR_NUMBER);
        bool connected = wm.autoConnect(apName.c_str(), "password");
        
        if (connected) {
            Serial.println("WiFi connected successfully!");
            Serial.print("IP address: ");
            Serial.println(WiFi.localIP());
            break;
        } else if (millis() - wifiConnectStart > WIFI_CONNECT_TIMEOUT) {
            Serial.println("WiFi connection timeout");
        }
        
        delay(1000);
    }
}

// setup Firebase 
void setupFirebase() {
    Serial.printf("Firebase Client v%s\n\n", FIREBASE_CLIENT_VERSION);

    //Firebase Configurations
    config.api_key = API_KEY;
    auth.user.email = USER_EMAIL;
    auth.user.password = USER_PASSWORD;
    config.database_url = DATABASE_URL;

    //Token status callback
    config.token_status_callback = tokenStatusCallback;

    //Initialize Firebase
    Firebase.begin(&config, &auth);
    Firebase.reconnectNetwork(true);

    //Set buffer sizes for better performance
    fbdo.setBSSLBufferSize(4096, 1024);

    //Set decimal precision
    Firebase.setDoubleDigits(5);

    //Wait for authentication
    Serial.print("Authenticating with Firebase...");
    while (auth.token.uid == "") {
      Serial.print(".");
      delay(1000);
    }
    Serial.println();
    Serial.println("Firebase authentication successful!");
    Serial.print("User UID: ");
    Serial.println(auth.token.uid.c_str());
    firebaseConnected = true;
}