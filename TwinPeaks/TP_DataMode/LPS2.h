#ifndef LPS2_H
#define LPS2_H
#include <Wire.h>
#include <Adafruit_LPS2X.h>
#include <Adafruit_Sensor.h>


class LPS2 {
    private:
    Adafruit_LPS22 lps;

    // usin SPI
    #define LPS_CS 21
    #define LPS_SCK 5
    #define LPS_MISO 19
    #define LPS_MOSI 18
    float pressure = -1;
    float temp = -1;

  public:
    void setup();
    void pullData();
    float getPressureHPa() const {return pressure; };
    float getAmbientTemperature() const {return temp; };
};

#endif // LPS2_H