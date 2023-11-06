#ifndef SEN55HANDLER_H
#define SEN55HANDLER_H

#include <SensirionI2CSen5x.h>

class Sen55Handler {
  private:
    SensirionI2CSen5x sen5x;
    struct Sen55Data {
      float massConcentrationPm1p0;
      float massConcentrationPm2p5;
      float massConcentrationPm4p0;
      float massConcentrationPm10p0;
      float ambientHumidity;
      float ambientTemperature;
      float vocIndex;
      float noxIndex;
    };

  Sen55Data* readSENData;

public:
    ~Sen55Handler(); // Destructor :,)

    char errorMessage[256];
    void setup();
    void pullData();
    float getPm2p5() const { return readSENData->massConcentrationPm2p5; }
    float getAmbientHumidity() const { return readSENData->ambientHumidity; }
    float getAmbientTemperature() const { return readSENData->ambientTemperature;}
};

#endif // SEN55HANDLER_H