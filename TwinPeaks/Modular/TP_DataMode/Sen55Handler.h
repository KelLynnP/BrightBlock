#ifndef SEN55HANDLER_H
#define SEN55HANDLER_H

#include <SensirionI2CSen5x.h>

class Sen55Handler {
private:

public:
    char errorMessage[256];
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
    Sen55Data pullData();
    void setup();

};

#endif // SEN55HANDLER_H