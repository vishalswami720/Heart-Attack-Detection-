#include <Wire.h> 
#include <MAX30105.h>
#include <ESP8266WiFi.h>
#include <WiFiClientSecure.h>  // Secure client for HTTPS

// Wi-Fi credentials
const char* ssid = "realme";
const char* password = "112233445566";

// HTTPS Server details
const char* host = "www.tejasswami.shop";  // Server domain
const int port = 443;  // HTTPS port

// Sensor object
MAX30105 particleSensor;
WiFiClientSecure client;  // Persistent client

void setup() {
  Serial.begin(115200);
  Wire.begin();

  // Connect to Wi-Fi
  WiFi.begin(ssid, password);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("\nWi-Fi connected");

  // Initialize MAX30105 sensor
  if (!particleSensor.begin(Wire, I2C_SPEED_STANDARD)) {
    Serial.println("Failed to initialize MAX30105 sensor!");
    while (1);
  } else {
    Serial.println("MAX30105 sensor initialized successfully");
    particleSensor.setup();
  }

  // Connect to the server once
  connectToServer();
}

void loop() {
  int bpm;

  // Check if a finger is placed on the sensor
  if (fingerDetected()) {
    bpm = random(60, 100);  // Generate BPM
    Serial.print("BPM: ");
    Serial.println(bpm);
  } else {
    Serial.println("No finger detected! Sending null.");
    bpm = -1;  // Representing null
  }

  // Send BPM data to server
  sendBPM(bpm);

  delay(1000); // 1-second delay before next check
}

// Function to check if a finger is placed on the sensor
bool fingerDetected() {
  int irValue = particleSensor.getIR();
  return irValue > 50000;  // Adjust threshold if needed
}

// Connect to server (persistent connection)
void connectToServer() {
  Serial.print("Connecting to server...");
  client.setInsecure();  // Bypass SSL verification
  if (!client.connect(host, port)) {
    Serial.println("Connection failed!");
  } else {
    Serial.println("Connected to server!");
  }
}

// Send BPM data to the server
void sendBPM(int bpm) {
  if (!client.connected()) {
    Serial.println("Reconnecting to server...");
    connectToServer();
  }

  // Create HTTP POST request
  String postData = (bpm == -1) ? "bpm=nun" : "bpm=" + String(bpm);
  
  client.print("POST /bpm/1737205139712 HTTP/1.1\r\n");
  client.print("Host: " + String(host) + "\r\n");
  client.print("Content-Type: application/x-www-form-urlencoded\r\n");
  client.print("Content-Length: " + String(postData.length()) + "\r\n");
  client.print("Connection: keep-alive\r\n\r\n");  // Keep connection open
  client.print(postData);

  Serial.println("Data sent: " + postData);
  
  // Read response (optional)
  while (client.available()) {
    String response = client.readString();
    Serial.println("Response: " + response);
  }
}
