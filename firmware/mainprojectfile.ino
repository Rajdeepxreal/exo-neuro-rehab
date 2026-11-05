#include <WiFi.h>
#include <WebSocketsServer.h>
#include <ArduinoJson.h>
#include <ESP32Servo.h>
#include <Wire.h>
#include <Adafruit_MPU6050.h>
#include <Adafruit_Sensor.h>

// --- NETWORK CREDENTIALS ---
const char* ssid = "ProjectWifi";
const char* password = "Raj123456";

// --- CORE OBJECTS ---
WebSocketsServer webSocket = WebSocketsServer(8765);
Servo exoServo;
Adafruit_MPU6050 mpu;

// --- HARDWARE CONFIG ---
const int servoPin = 18;

// --- MOTOR CONTROL STATE ---
int currentAngle = 0;
int targetAngle = 0;
int speedDelay = 15; 
unsigned long lastServoMove = 0;
unsigned long lastTelemetrySend = 0;

void webSocketEvent(uint8_t num, WStype_t type, uint8_t * payload, size_t length) {
  if (type == WStype_TEXT) {
    String msg = String((char*)payload);
    
    if (msg.startsWith("A:")) {
      targetAngle = msg.substring(2).toInt();
      if (targetAngle < 0) targetAngle = 0;
      if (targetAngle > 180) targetAngle = 180;
    }
    else if (msg.startsWith("S:")) {
      speedDelay = msg.substring(2).toInt();
    }
  }
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22);
  
  // 1. Setup Servo
  ESP32PWM::allocateTimer(0);
  exoServo.setPeriodHertz(50);
  exoServo.attach(servoPin, 500, 2400);
  exoServo.write(currentAngle);

  // 2. Init MPU6050 (Default Address 0x68)
  if (!mpu.begin(0x68)) {
    Serial.println("❌ MPU6050 missing! Check D21/D22 wiring.");
  } else {
    mpu.setAccelerometerRange(MPU6050_RANGE_4_G);
    mpu.setGyroRange(MPU6050_RANGE_500_DEG);
    mpu.setFilterBandwidth(MPU6050_BAND_21_HZ);
    Serial.println("✅ MPU6050 initialized.");
  }

  // 3. Robust Wi-Fi Connect
  Serial.println("\nConnecting to Wi-Fi...");
  WiFi.mode(WIFI_STA);
  WiFi.disconnect();
  delay(100);
  
  WiFi.begin(ssid, password);
  int attempts = 0;
  while (WiFi.status() != WL_CONNECTED && attempts < 20) {
    delay(500);
    Serial.print(".");
    attempts++;
  }

  if (WiFi.status() == WL_CONNECTED) {
    Serial.println("\n--- SYSTEM ONLINE ---");
    Serial.print("Enter this IP in your HTML file: ");
    Serial.println(WiFi.localIP());
  } else {
    Serial.println("\n❌ Wi-Fi Failed! Check your 2.4GHz hotspot settings.");
  }

  // 4. Start Server
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);
}

void loop() {
  webSocket.loop(); 

  // --- NON-BLOCKING SERVO SWEEP ---
  if (millis() - lastServoMove >= speedDelay) {
    if (currentAngle < targetAngle) {
      currentAngle++;
      exoServo.write(currentAngle);
    } else if (currentAngle > targetAngle) {
      currentAngle--;
      exoServo.write(currentAngle);
    }
    lastServoMove = millis();
  }

  // --- TELEMETRY STREAMING (30Hz) ---
  if (millis() - lastTelemetrySend >= 33) { 
    sensors_event_t a, g, temp;
    mpu.getEvent(&a, &g, &temp);

    // Simplified JSON Payload
    StaticJsonDocument<200> doc;
    doc["ax"] = a.acceleration.x;
    doc["ay"] = a.acceleration.y;
    doc["az"] = a.acceleration.z;
    doc["gx"] = g.gyro.x;
    doc["gy"] = g.gyro.y;
    doc["gz"] = g.gyro.z;

    String jsonString;
    serializeJson(doc, jsonString);
    webSocket.broadcastTXT(jsonString);
    lastTelemetrySend = millis();
  }
}