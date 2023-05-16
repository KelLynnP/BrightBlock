#include <WiFi.h>
#include <Adafruit_GPS.h>
#include <SoftwareSerial.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME680.h>
#include <Adafruit_ICM20948.h>

///~~~~~~~~~~~~~~~~~~  User input info! ~~~~~~~~~~~~~~~~~~  //
const char* ssid ="GparksB"; // "Prosperity";
const char* password = "4rustypaintcan"; //"f0revery0ne"; // 
const char* serverName = "https://bb-vercel-kellynnp.vercel.app/api/hello";
int SamplingRate = 10000; // .1 Hz freqnecy

///~~~~~~~~~~~~~~~~~~  Variable and Function definitions  ~~~~~~~~~~~~~~~~~~  //
#define GPSSerial Serial1
Adafruit_GPS GPS(&GPSSerial);
#define GPSECHO false
Adafruit_BME680 bme; // Initialize the BME688 sensor object
Adafruit_ICM20948 icm; // Initialize the ICM209448 sensor object
uint32_t GpsTimer = millis();
uint32_t GlobalTimer = millis();
const int buttonPin = 25;  // the number of the pushbutton pin
int buttonState = 0;


struct BMEData{
  float temperature;
  float humidity;
  float pressure;
};

struct ICMData{
  float AccelX;  // acceleration
  float AccelY; 
  float AccelZ; 

  float MagX; // no units??
  float MagY; 
  float MagZ;

  float GyroX;  // radians/s 
  float GyroY ;
  float GyroZ ;
};

struct GPSData{
  char TimeStamp[20];
  float latitude;
  float longitude;
  float altitude;
};

ICMData getICMData(){
  ICMData ICM;
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;
  icm.getEvent(&accel, &gyro, &temp, &mag);
  // GTemp = temp.temperature; // not pulling a second temperature value but we could

  ICM.AccelX = accel.acceleration.x; // acceleration
  ICM.AccelY = accel.acceleration.y;
  ICM.AccelZ = accel.acceleration.z;

  ICM.MagX = mag.magnetic.x; // no units??
  ICM.MagY = mag.magnetic.y;
  ICM.MagZ = mag.magnetic.z;

  ICM.GyroX = gyro.gyro.x; // radians/s 
  ICM.GyroY = gyro.gyro.y;
  ICM.GyroZ = gyro.gyro.z;
  // Serial.print(ICM.GyroX);
  return ICM;
}

BMEData getBMEData() {
  BMEData BME;
  BME.temperature = bme.readTemperature();
  BME.humidity = bme.readHumidity();
  BME.pressure = bme.readPressure() / 100.0F;
  return BME;
}

void postData(char TimeStamp[20], float temperature, float humidity, float pressure, float latitude, float longitude, float altitude, float AccelX, float AccelY, float AccelZ, float GyroX,  float GyroY, float GyroZ) {
  if (WiFi.status() == WL_CONNECTED) {
    HTTPClient http;
    http.begin(serverName);
    http.addHeader("Content-Type", "application/json");
    DynamicJsonDocument jsonDoc(1024);
    jsonDoc["TimeStamp"] = TimeStamp;
    jsonDoc["temperature"] = temperature;
    jsonDoc["humidity"] = humidity;
    jsonDoc["pressure"] = pressure;
    jsonDoc["latitude"] = latitude;
    jsonDoc["longitude"] = longitude;
    jsonDoc["altitude"] = altitude;
    jsonDoc["AccelX"] = AccelX;
    jsonDoc["AccelY"] = AccelY;
    jsonDoc["AccelZ"] = AccelZ;
    jsonDoc["GyroX"] = GyroX;
    jsonDoc["GyroY"] = GyroY;
    jsonDoc["GyroZ"] = GyroZ;
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
  memset(TempGPS.TimeStamp, -1, sizeof(TempGPS.TimeStamp));  
  TempGPS.latitude = -1;
  TempGPS.longitude = -1;
  TempGPS.altitude = -1;
  char c = GPS.read(); // BECAUSE OF GPS LOGIC THIS NEEDS TO BE CALLED AT LEAST TWICE A SECOND... This means global delay functions are OUT
  // Serial.print(c); // SHOWS ALL THE NMEA STRINGS! PRINT AT UR OWN RISK :) 
  if (GPSECHO)
    if (c) Serial.print(c); // if a sentence is received, we can check the checksum, parse it...
  if (GPS.newNMEAreceived()) { // a tricky thing here is if we print the NMEA sentence, or data // we end up not listening and catching other sentences! // so be very wary if using OUTPUT_ALLDATA and trying to print out data
    // Serial.print(GPS.lastNMEA()); // this also sets the newNMEAreceived() flag to false
    if (!GPS.parse(GPS.lastNMEA())) // this also sets the newNMEAreceived() flag to false
      return TempGPS; // we can fail to parse a sentence in which case we should just wait for another
  }

  if (millis() - GpsTimer > 2000) {   // aprox save every 2
    GpsTimer = millis(); // reset the timer
    char timestamp[20];     // get the current time from the GPS and format it into the timestamp variable
    sprintf(timestamp, "%02d:%02d:%02d %02d/%02d/%04d", GPS.hour, GPS.minute, GPS.seconds, GPS.day, GPS.month, GPS.year);
    // ~~~~~~~~~~~~~~~~~~~ Run these lines to debug ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~// 
    // Serial.print("Fix: "); Serial.print((int)GPS.fix);
    // Serial.print(" quality: "); Serial.println((int)GPS.fixquality);
    if (GPS.fix) {
      TempGPS.latitude = GPS.latitude;
      TempGPS.longitude = GPS.longitude;
      TempGPS.altitude = GPS.altitude;
      // Serial.print(TempGPS.latitude);
    }
  }
  return TempGPS;
}

void setup() {
  Serial.begin(115200);
  pinMode(buttonPin, INPUT);
  // Wifi Set up
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  // GPS Store
  GPSSerial.begin(9600); //9600 Default buad rate for Ultimate GPS
  GPS.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  GPS.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ); // 1 Hz update rate

  if (!bme.begin()) {
    Serial.println("Could not find BME688 sensor!");
  }
  else{    // ~if~ we find Set up oversampling and filter initialization

    bme.setTemperatureOversampling(BME680_OS_8X);
    bme.setHumidityOversampling(BME680_OS_2X);
    bme.setPressureOversampling(BME680_OS_4X);
    bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
    bme.setGasHeater(320, 150); // 320*C for 150 ms
  }

  if (!icm.begin_I2C()) { // if (!icm.begin_SPI(ICM_CS)) { // if (!icm.begin_SPI(ICM_CS, ICM_SCK, ICM_MISO, ICM_MOSI)) {

    Serial.println("Failed to find ICM20948 chip");
    while (1) {
      delay(10);
    }
  }
  else{
  Serial.println("ICM20948 Found!");
  // icm.setAccelRange(ICM20948_ACCEL_RANGE_16_G);
  Serial.print("Accelerometer range set to: ");
  switch (icm.getAccelRange()) {
  case ICM20948_ACCEL_RANGE_2_G:
    Serial.println("+-2G");
    break;
  case ICM20948_ACCEL_RANGE_4_G:
    Serial.println("+-4G");
    break;
  case ICM20948_ACCEL_RANGE_8_G:
    Serial.println("+-8G");
    break;
  case ICM20948_ACCEL_RANGE_16_G:
    Serial.println("+-16G");
    break;
  }
  // Serial.println("OK");
  // icm.setGyroRange(ICM20948_GYRO_RANGE_2000_DPS);
  Serial.print("Gyro range set to: ");
  switch (icm.getGyroRange()) {
  case ICM20948_GYRO_RANGE_250_DPS:
    Serial.println("250 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_500_DPS:
    Serial.println("500 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_1000_DPS:
    Serial.println("1000 degrees/s");
    break;
  case ICM20948_GYRO_RANGE_2000_DPS:
    Serial.println("2000 degrees/s");
    break;
  }

  //  icm.setAccelRateDivisor(4095);
  uint16_t accel_divisor = icm.getAccelRateDivisor();
  float accel_rate = 1125 / (1.0 + accel_divisor);

  Serial.print("Accelerometer data rate divisor set to: ");
  Serial.println(accel_divisor);
  Serial.print("Accelerometer data rate (Hz) is approximately: ");
  Serial.println(accel_rate);

  //  icm.setGyroRateDivisor(255);
  uint8_t gyro_divisor = icm.getGyroRateDivisor();
  float gyro_rate = 1100 / (1.0 + gyro_divisor);

  Serial.print("Gyro data rate divisor set to: ");
  Serial.println(gyro_divisor);
  Serial.print("Gyro data rate (Hz) is approximately: ");
  Serial.println(gyro_rate);

  // icm.setMagDataRate(AK09916_MAG_DATARATE_10_HZ);
  Serial.print("Magnetometer data rate set to: ");
  switch (icm.getMagDataRate()) {
  case AK09916_MAG_DATARATE_SHUTDOWN:
    Serial.println("Shutdown");
    break;
  case AK09916_MAG_DATARATE_SINGLE:
    Serial.println("Single/One shot");
    break;
  case AK09916_MAG_DATARATE_10_HZ:
    Serial.println("10 Hz");
    break;
  case AK09916_MAG_DATARATE_20_HZ:
    Serial.println("20 Hz");
    break;
  case AK09916_MAG_DATARATE_50_HZ:
    Serial.println("50 Hz");
    break;
  case AK09916_MAG_DATARATE_100_HZ:
    Serial.println("100 Hz");
    break;
  }
  Serial.println();
  }

}

void loop() // run over and over again
{
  GPSData ValidGPSData;
  GPSData MaybeNullGPSData = GPS_ConstantReadNStore(); //yelling into void! Hello GPS are you still there 
  buttonState = digitalRead(buttonPin);
  // Serial.println(buttonState);
  // Serial.println(MaybeNullGPSData.latitude);
  if (MaybeNullGPSData.latitude != -1.0){ // Flag for not set to a true value
  ///___________  FIXME: THIS LINE DOES NOT APPEAR TO BE WORKING WELL _________________
    ValidGPSData = MaybeNullGPSData; 
    // Serial.println(ValidGPSData.latitude);
  }
 // Pull sample rlate
  if (millis() - GlobalTimer > SamplingRate ) {   // aprox save every 2
    GlobalTimer = millis(); // reset the timer
    BMEData BME = getBMEData();
    ICMData ICM = getICMData();
    // postData(ValidGPSData.TimeStamp, BME.temperature, BME.humidity, BME.pressure, ValidGPSData.latitude, ValidGPSData.longitude, ValidGPSData.altitude, ICM.AccelX, ICM.AccelY, ICM.AccelZ, ICM.GyroX, ICM.GyroY, ICM.GyroZ);
  }
}
