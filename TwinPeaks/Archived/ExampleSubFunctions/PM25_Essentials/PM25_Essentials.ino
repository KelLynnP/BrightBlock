#include "Adafruit_PM25AQI.h"

Adafruit_PM25AQI aqi = Adafruit_PM25AQI();

void setup() {
  // Wait for serial monitor to open
  Serial.begin(115200);
  while (!Serial) delay(10);

  Serial.println("Adafruit PMSA003I Air Quality Sensor");

  // Wait one second for sensor to boot up!
  delay(1000);
  if (! aqi.begin_I2C()) {      // connect to the sensor over I2C
    Serial.println("Could not find PM 2.5 sensor!");
    while (1) delay(10);
  }
  Serial.println("PM25 found!");
}

void loop() {
  PM25_AQI_Data data;
  if (! aqi.read(&data)) {
    Serial.println("Could not read from AQI");
    delay(500);  // try again in a bit!
    return;
  }
  Serial.print(F("PM 2.5: ")); Serial.print(data.pm25_env); Serial.println(" microG/m^3");
  Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  delay(500);
}

// other types of data pulling
  // Serial.println("AQI reading success");

  // Serial.println();
  // Serial.println(F("---------------------------------------"));
  // Serial.println(F("Concentration Units (standard)"));
  // Serial.println(F("---------------------------------------"));
  // Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_standard);
  // Serial.print(F("\t\tPM 2.5: ")); Serial.print(data.pm25_standard);
  // Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_standard);
  // Serial.println(F("Concentration Units (environmental)"));
  // Serial.println(F("---------------------------------------"));
  // Serial.print(F("PM 1.0: ")); Serial.print(data.pm10_env);
  // Serial.print(F("\t\tPM 10: ")); Serial.println(data.pm100_env);
  // Serial.println(F("---------------------------------------"));
  // Serial.print(F("Particles > 0.3um / 0.1L air:")); Serial.println(data.particles_03um);
  // Serial.print(F("Particles > 0.5um / 0.1L air:")); Serial.println(data.particles_05um);
  // Serial.print(F("Particles > 1.0um / 0.1L air:")); Serial.println(data.particles_10um);
  // Serial.print(F("Particles > 2.5um / 0.1L air:")); Serial.println(data.particles_25um);
  // Serial.print(F("Particles > 5.0um / 0.1L air:")); Serial.println(data.particles_50um);
  // Serial.print(F("Particles > 10 um / 0.1L air:")); Serial.println(data.particles_100um);
  // Serial.println(F("---------------------------------------"));
  