#include <WiFi.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>

// User input info!
const char* ssid ="GparksB"; // "Prosperity";
const char* password = "4rustypaintcan"; //"f0revery0ne"; // 
const char* serverName = "https://bb-vercel-kellynnp.vercel.app/api/hello";

//Variable and Function definitions 
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
Adafruit_BME680 bme; // Initialize the BME688 sensor object
uint32_t timer = millis();
// Adafruit_GPS GPS(&Serial1);

struct BMEData{
  float temperature;
  float humidity;
  float pressure;
};

struct GPSData{
  char TimeStamp[20];
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

GPSData GPS_ConstantReadNStore(){
  GPSData TempGPS;
  memset(TempGPS.TimeStamp, 0, sizeof(TempGPS.TimeStamp));  TempGPS.latitude = 0;
  TempGPS.longitude = 0;
  TempGPS.altitude = 0;
  char c = GPS.read(); // BECAUSE OF GPS LOGIC THIS NEEDS TO BE CALLED AT LEAST TWICE A SECOND... This means global delay functions are OUT
  // Serial.print(c); // if you want to debug, this is a good time to do it!
  if (GPSECHO)
    if (c) Serial.print(c); // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) { // a tricky thing here is if we print the NMEA sentence, or data // we end up not listening and catching other sentences! // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return TempGPS; // we can fail to parse a sentence in which case we should just wait for another
  }

  if (millis() - timer > 2000) {   // aprox save every 2
    timer = millis(); // reset the timer
    char timestamp[20];     // get the current time from the GPS and format it into the timestamp variable
    sprintf(timestamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
    // Serial.print("Fix: "); Serial.print((int)GPS.fix);
    // Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      TempGPS.latitude = GPS.latitude;
      TempGPS.longitude = GPS.longitude;
      TempGPS.altitude = GPS.altitude;
      // Serial.print(TempGPS.altitude);
    }
  }
  return TempGPS;
}

void setup() {
  Serial.begin(115200);
  // Wifi Set up
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");

  GPSSerial.begin(9600); //9600 Default buad rate for Ultimate GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate

  if (!bme.begin()) {
    Serial.println("Could not find BME688 sensor!");
  }

}
void loop() // run over and over again
{
  // Set timer
 //----------------------------------


  // GPS DATA LOGGING CONSTANTLY
  // Set gps data to a temp variable
  GPS_ConstantReadNStore();// pull new data
  // check to see if is null or not 
  // replace with temp variable if real data

 // Pull sample rlate

 // if frequency then pull a bunch of sensor data 
  //----------------------------------

  // BMEData BME; 
  // BME = getBMEData();
  

}