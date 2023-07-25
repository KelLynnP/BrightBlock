#include <Wire.h>
#include <Adafruit_Sensor.h>
#include "Adafruit_BME680.h"
#include "Adafruit_VL53L1X.h"
#include <Adafruit_ICM20948.h>
//#include "FastLED.h"

#define IRQ_PIN 2
#define XSHUT_PIN 3

#define SEALEVELPRESSURE_HPA (1013.25)
Adafruit_VL53L1X vl53 = Adafruit_VL53L1X(XSHUT_PIN, IRQ_PIN);
Adafruit_BME680 bme; // I2C

// Accelerometer data
Adafruit_ICM20948 icm;
uint16_t measurement_delay_us = 65535; // Delay between measurements for testing
// For SPI mode, we need a CS pin
#define ICM_CS 10
// Modified + Cleanded

float humidity, HTemp, pressure, distance, altitude, GTemp; 

void setup() {
  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~Humidity CODE~~~~~~~~~~~~~~~~~~~~~~~~~~
  Serial.begin(9600);
  while (!Serial);
  Serial.println(F("BME680 test"));

  if (!bme.begin()) {
    Serial.println("Could not find a valid BME680 sensor, check wiring!");
    while (1);
  }

  int led = 21;
  pinMode(led, OUTPUT);

  // Set up oversampling and filter initialization
  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150); // 320*C for 150 ms

  // ~~~~~~~~~~~~~~~~~~~~~~~~~~~Distance Code CODE~~~~~~~~~~~~~~~~~~~~~~~~~~
  Wire.begin();
  if (! vl53.begin(0x29, &Wire)) {
    Serial.print(F("Error on init of VL sensor: "));
    Serial.println(vl53.vl_status);
    while (1)       delay(10);
  }
  Serial.println(F("VL53L1X sensor OK!"));

  Serial.print(F("Sensor ID: 0x"));
  Serial.println(vl53.sensorID(), HEX);

  if (! vl53.startRanging()) {
    Serial.print(F("Couldn't start ranging: "));
    Serial.println(vl53.vl_status);
    while (1)       delay(10);
  }
  Serial.println(F("Ranging started"));

  // Valid timing budgets: 15, 20, 33, 50, 100, 200 and 500ms!
  vl53.setTimingBudget(50);
  Serial.print(F("Timing budget (ms): "));
  Serial.println(vl53.getTimingBudget());

 // ~~~~~~~~~~~~~~~~~~~~~~~~~~~Gyroscope Code ~~~~~~~~~~~~~~~~~~~~~~~~~~
  if (!icm.begin_I2C()) {
    // if (!icm.begin_SPI(ICM_CS)) {
    // if (!icm.begin_SPI(ICM_CS, ICM_SCK, ICM_MISO, ICM_MOSI)) {

    Serial.println("Failed to find ICM20948 chip");
    while (1) {
      delay(10);
    }
  }
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
  Serial.println("OK");

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

void loop() {
  // More Humidity Data !!

  // BME Data
  if (! bme.performReading()) {
    Serial.println("Failed to perform reading : on BME(");
  }
  HTemp = bme.temperature; // C
  pressure = bme.pressure / 100.0; // " hPa"
  humidity= bme.humidity;  // " %"
  
  // Pulling Sensor data from Gyroscope
  sensors_event_t accel;
  sensors_event_t gyro;
  sensors_event_t mag;
  sensors_event_t temp;

  // Distance data 
  int16_t distance;

  if (vl53.dataReady()) {
    // new measurement for the taking!
    distance = vl53.distance();
    if (distance == -1) {
      // something went wrong!
      Serial.print(F("Couldn't get distance: "));
      Serial.println(vl53.vl_status);
    }


    // data is read out, time for another reading!
    vl53.clearInterrupt();
  }
  // Gyroscope  
  icm.getEvent(&accel, &gyro, &temp, &mag);
  GTemp = temp.temperature; 

  float AccelX = accel.acceleration.x; // acceleration
  float AccelY = accel.acceleration.y;
  float AccelZ = accel.acceleration.z;

  float MagX = mag.magnetic.x; // no units??
  float MagY = mag.magnetic.y;
  float MagZ = mag.magnetic.z;

  float GyroX = gyro.gyro.x; // radians/s 
  float GyroY = gyro.gyro.y;
  float GyroZ = gyro.gyro.z;

  // print statments 
  Serial.print(F("Distance: "));
  Serial.print(distance);
  Serial.println(" mm");

  Serial.print("Temperature = ");
  Serial.print(HTemp);
  Serial.println(" *C");

  Serial.print("Pressure = ");
  Serial.print(pressure);
  Serial.println();

  Serial.print("Humidity = ");
  Serial.print(humidity);
  Serial.println(" %");

  Serial.print("Gas = ");
  Serial.print(bme.gas_resistance / 1000.0);
  Serial.println(" KOhms");

  Serial.print("Approx. Altitude = ");
  Serial.print(bme.readAltitude(SEALEVELPRESSURE_HPA));
  Serial.println(" m");

  Serial.println();

  Serial.print("Temperature ");
  Serial.print(GTemp);
  Serial.println(" deg C");

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("Accel X: ");
  Serial.print(AccelX);
  Serial.print("Y: ");
  Serial.print(AccelY);
  Serial.print("Z: ");
  Serial.print(AccelZ);
  Serial.println(" m/s^2 ");

  Serial.print("Mag X: ");
  Serial.print(MagX);
  Serial.print("Y: ");
  Serial.print(MagY);
  Serial.print("Z: ");
  Serial.print(MagZ);
  Serial.println(" uT");

  /* Display the results (acceleration is measured in m/s^2) */
  Serial.print("tGyro X: ");
  Serial.print(GyroX);
  Serial.print("Y: ");
  Serial.print(GyroY);
  Serial.print("Z: ");
  Serial.print(GyroZ);
  Serial.println(" radians/s ");
  Serial.println();

  delay(500);
}