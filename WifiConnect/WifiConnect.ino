#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// User input info!
const char* ssid = "GparksB";
const char* password = "4rustypaintcan";
const char* serverName = "https://bb-vercel-kellynnp.vercel.app/api/hello";


//Variable and Function definitions 
Adafruit_BME680 bme; // Initialize the BME688 sensor object
Adafruit_GPS GPS(&Serial1);
struct BMEData{
  float temperature;
  float humidity;
  float pressure;
};

struct GPSData{
  float latitude;
  float longitude;
  float altitude;
};

BMEData getBMEData() {
  BMEData BME;
  BME.temperature = bme.readTemperature();
  BME.humidity = bme.readHumidity();
  BME.pressure = bme.readPressure() / 100.0F;
  return BME;
}

GPSData getGPSData() {
  GPSData GPS_Data;
  char c = GPS.read();
  if (GPS.newNMEAreceived()) {
    if (!GPS.parse(GPS.lastNMEA())) {
      return GPS_Data;
    }
  }
  if (GPS.fix) {
    GPS.latitude = GPS.latitudeDegrees;
    GPS.longitude = GPS.longitudeDegrees;
    GPS.altitude = GPS.altitude / 100.0F;
  }
  return GPS_Data;
}

void printGPSData() {
  int count = 0;
  while (Serial1.available()) {
    if (count > 10 ){
      count = count + 1;
      char c = Serial1.read();
      Serial.print(c);
      delay(2000);
    }
    else {
        Serial.print("Hitting count");
      return;
    }
  }
  Serial.print("Sure not finding serial 1 available");

}

void postData(int randomNumber, float temperature, float humidity, float pressure, float latitude, float longitude, float altitude) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["value"] = randomNumber;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["pressure"] = pressure;
    jsonDoc["latitude"] = latitude;
    jsonDoc["longitude"] = longitude;
    jsonDoc["altitude"] = altitude;
    String postData;
    serializeJson(jsonDoc, postData);
    Serial.print(postData);
    Serial.println();
    int httpResponseCode = http.POST(postData);
    if (httpResponseCode > 0) {
      Serial.print("HTTP Response code: ");
      Serial.println(httpResponseCode);
      String response = http.getString();
      Serial.println(response);
    } else {
      Serial.print("Error in posting data. HTTP Error code: ");
      Serial.println(httpResponseCode);
    }
    http.end();
  } else {
    Serial.println("WiFi Disconnected");
  }
}

int generateRand(){
  return random(0, 100); // generate a random number between 0 and 100
}

void setup() {
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  if (!bme.begin()) {
    Serial.println("Could not find BME688 sensor!");
  }
}

void loop() {
  // int randomNumber = generateRand();

  // BMEData BME; 
  // BME = getBMEData();

  // GPSData GPS;
  // GPS = getGPSData();
  printGPSData();
  // postData(randomNumber, BME.temperature, BME.humidity, BME.pressure, GPS.latitude, GPS.longitude, GPS.altitude);
  delay(2000);
}


