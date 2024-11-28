#include "sen55Handler.h"

Sen55Handler::Sen55Handler() : data(), errorMessage() {
}

void Sen55Handler::setup() {
    Wire.begin();
    sen5x.begin(Wire);
    uint16_t error;
    error = sen5x.startMeasurement(); 
    if (error) {
      Serial.print("Error trying to execute startMeasurement(): ");
      errorToString(error, errorMessage, 256);
      Serial.println(errorMessage);
    }
}

bool Sen55Handler::pullData() {
    uint16_t error = sen5x.readMeasuredValues(
        data.massConcentrationPm1p0, data.massConcentrationPm2p5, 
        data.massConcentrationPm4p0, data.massConcentrationPm10p0, 
        data.ambientHumidity, data.ambientTemperature, 
        data.vocIndex, data.noxIndex);

    if (error) {
        printError(error);
        return false;
    }
    return true;
}

float Sen55Handler::getPm2p5() const {
    return data.massConcentrationPm2p5;
}

float Sen55Handler::getAmbientHumidity() const {
    return data.ambientHumidity;
}

float Sen55Handler::getAmbientTemperature() const {
    return data.ambientTemperature;
}

float Sen55Handler::getVocIndex() const {
    return data.vocIndex;
}

float Sen55Handler::getNoxIndex() const {
    return data.noxIndex;
}


void Sen55Handler::printError(uint16_t error) {
    Serial.print("Error trying to execute readMeasuredValues(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
}