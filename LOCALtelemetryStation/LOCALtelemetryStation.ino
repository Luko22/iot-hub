// THIS IS FOR LOCAL DEVICES IN WHICH YOU MQTT BROKER AND ESP32 ARE CONNECTED TO THE SAME NETWORK
#include <WiFi.h>
#include <PubSubClient.h>
#include <ArduinoJson.h>
#include "DHT.h"
#include "credentials.h"  // for private ssid and passwords

// ===============================================================
// HARDWARE CONFIGURATION
// ===============================================================
HardwareSerial gpsSerial(1);  // UART1 for GPS

#define GPS_RX 16  // ESP32 RX ← GPS TX
#define GPS_TX 17  // ESP32 TX → GPS RX (optional)
#define GPS_BAUD 9600

// Pin configuration and sensor type
#define DHTPIN 4       // GPIO pin connected to DHT22
#define DHTTYPE DHT22  // Specify DHT22 sensor

DHT dht(DHTPIN, DHTTYPE);

#define LEDPIN 15
// ===============================================================
// NETWORK CONFIGURATION (WiFi)
// ===============================================================
const char* ssid = mySSID;
const char* password = myPASSWORD;

// =================================================
//MQTT
// =================================================
const char* mqttServer = "10.215.192.93";  // <-- CHECK BROKER IP EVRYTIME BC IT CAN CHANGE
const int mqttPort = 1883;

const char* mqttClientID = "espKleveNeo7M";
const char* mqttTopic = "iot/espKleveNeo7M/telemetry";



const uint16_t updateInterval = 10;  // seconds
const bool enableOfflineStorage = true;
const uint8_t offlineBufferLimit = 20;
const bool enableAutoReconnect = true;

// Timezone
const int8_t timeOffsetHours = 5;
const int8_t timeOffsetMinutes = 30;

// ===============================================================

WiFiClient espClient;
PubSubClient mqttClient(espClient);


// ===============================================================
// INTERNAL STATE
// ===============================================================

uint32_t sensorStartTime = 0;

struct GPS_Coordinates {
  double latitude;
  double longitude;
  bool valid;
};

GPS_Coordinates parseGPRMC(String nmea) {
  GPS_Coordinates coord;
  coord.valid = false;

  if (nmea.startsWith("$GPRMC")) {
    // Split by comma
    int parts[12];
    String tokens[12];
    int idx = 0;
    int start = 0;
    for (int i = 0; i < nmea.length() && idx < 12; i++) {
      if (nmea[i] == ',' || nmea[i] == '*') {
        tokens[idx++] = nmea.substring(start, i);
        start = i + 1;
      }
    }

    if (tokens[2] == "A") {  // Status = A means valid fix
      coord.valid = true;
      // Latitude
      double lat = tokens[3].toDouble();
      double lat_deg = floor(lat / 100.0);
      double lat_min = lat - lat_deg * 100.0;
      coord.latitude = lat_deg + lat_min / 60.0;
      if (tokens[4] == "S") coord.latitude = -coord.latitude;

      // Longitude
      double lon = tokens[5].toDouble();
      double lon_deg = floor(lon / 100.0);
      double lon_min = lon - lon_deg * 100.0;
      coord.longitude = lon_deg + lon_min / 60.0;
      if (tokens[6] == "W") coord.longitude = -coord.longitude;
    }
  }
  return coord;
}

void connectToWiFi() {
  if (WiFi.status() == WL_CONNECTED) return;

  Serial.print("Connecting to WiFi...");
  WiFi.begin(ssid, password);

  uint32_t start = millis();
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
    if (millis() - start > 15000) {
      Serial.println("\nWiFi timeout");
      return;
    }
  }

  Serial.println("\nWiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void connectToMQTT() {
  if (mqttClient.connected()) return;

  Serial.print("Connecting to MQTT...");
  if (mqttClient.connect(mqttClientID)) {
    Serial.println(" connected");
  } else {
    Serial.print(" failed, rc=");
    Serial.println(mqttClient.state());
  }
}



// ===============================================================
// SETUP
// ===============================================================
void setup() {

  Serial.begin(115200);
  
  delay(1000);

  dht.begin();
  sensorStartTime = millis();

  pinMode(LEDPIN, OUTPUT);

  Serial.println("\n=== MQTT TELEMETRY STATION ===");

  // ---- GPS UART ----
  gpsSerial.begin(GPS_BAUD, SERIAL_8N1, GPS_RX, GPS_TX);
  Serial.println("GPS serial initialized (UART1 @ 9600)");

  connectToWiFi();
  Serial.println(ssid);

  
  mqttClient.setServer(mqttServer, mqttPort);
  ///first temp and hum

  // ---- GPS Warm-up ----
  // gpsStartTime = millis();
  Serial.println("\nWaiting for GPS fix...");
  Serial.println("→ Move device outdoors with clear sky view");
  Serial.println("→ First fix may take 2–5 minutes\n");
}
// ====================================================
void publishTelemetry(double lat, double lon, float temp, float hum) {
  StaticJsonDocument<256> doc;

  doc["device_id"] = mqttClientID;
  doc["lat"] = lat;
  doc["lon"] = lon;
  doc["temperature"] = temp;
  doc["humidity"] = hum;
  

  char buffer[256];
  size_t n = serializeJson(doc, buffer);

  mqttClient.publish(mqttTopic, buffer, n);
}



// ===============================================================
// LOOP
// ===============================================================
void loop() {
  connectToWiFi();
  connectToMQTT();
  mqttClient.loop();


  // Read humidity and temperature data
  float humidity = dht.readHumidity();
  float temperature = dht.readTemperature();

  // Error handling if sensor fails to provide data
  if (isnan(humidity) || isnan(temperature)) {
    Serial.println("Error: Unable to read data from DHT sensor.");
    return;
  }


  static double lastLat = 0, lastLon = 0;

  static String nmeaLine = "";
  while (gpsSerial.available()) {
    char c = gpsSerial.read();
    if (c == '\n') {
      GPS_Coordinates coord = parseGPRMC(nmeaLine);
      if (coord.valid) {
        Serial.println();
        Serial.println();
        Serial.print("Humidity: ");
        Serial.print(humidity);
        Serial.print(" %\t");
        Serial.print("|Temperature: ");
        Serial.print(temperature);
        Serial.println(" °C | ");
        Serial.print("LAT: ");
        Serial.print(coord.latitude, 6);
        Serial.print(", LON: ");
        Serial.println(coord.longitude, 6);
        Serial.println();
        Serial.println();

        publishTelemetry(coord.latitude, coord.longitude, temperature, humidity);
      }
      nmeaLine = "";
    } else if (c != '\r') {
      nmeaLine += c;

    }
  }

  // Print sesnor results to the serial monitor evry x milliseconds
  // if (millis() - sensorStartTime > 10000) {
  //   Serial.println();
  //   Serial.println();
  //   Serial.print("Humidity: ");
  //   Serial.print(humidity);
  //   Serial.print(" %\t");
  //   Serial.print("|Temperature: ");
  //   Serial.print(temperature);
  //   Serial.println(" °C | ");
  //   Serial.print("NEW FIX → LAT: ");
  //   Serial.print(coord.latitude, 6);
  //   Serial.print(", LON: ");
  //   Serial.println(coord.longitude, 6);

  //   Serial.println();
  //   Serial.println();
  //   dhtStartTime = millis(); //resets time
  // }

  
}

















void signal(int pin) {
  digitalWrite(pin, HIGH);
  delay(50);
  digitalWrite(pin, LOW);
  delay(50);
  digitalWrite(pin, HIGH);
  delay(50);
  digitalWrite(pin, LOW);
  delay(50);
  digitalWrite(pin, HIGH);
  delay(50);
  digitalWrite(pin, LOW);
  delay(50);
  digitalWrite(pin, HIGH);
  delay(50);
  digitalWrite(pin, LOW);
  delay(50);
}