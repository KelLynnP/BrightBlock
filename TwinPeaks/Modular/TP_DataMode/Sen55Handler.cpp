#include "Sen55Handler.h"

void Sen55Handler::setup(){
   // Sensirion Data Rhings The used commands use up to 48 bytes. On some Arduino's the default buffer space is not large enough
    #define MAXBUF_REQUIREMENT 48

    #if (defined(I2C_BUFFER_LENGTH) && (I2C_BUFFER_LENGTH >= MAXBUF_REQUIREMENT)) || (defined(BUFFER_LENGTH) && BUFFER_LENGTH >= MAXBUF_REQUIREMENT)
    #define USE_PRODUCT_INFO
    #endif
    
    readSENData = nullptr;  // Initialize the pointer to nullptr, just to be safe.
}

Sen55Handler::~Sen55Handler() {
    delete readSENData;  // Delete the allocated memory
}


void Sen55Handler::pullData(){
  char errorMessage[256];
  uint16_t error;

  Sen55Handler::Sen55Data returnData;
  
  float massConcentrationPm1p0;
  float massConcentrationPm2p5;
  float massConcentrationPm4p0;
  float massConcentrationPm10p0;
  float ambientHumidity;
  float ambientTemperature;
  float vocIndex;
  float noxIndex;

  error = sen5x.readMeasuredValues(
    massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
    massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
    noxIndex);
          
    readSENData->massConcentrationPm1p0 = massConcentrationPm1p0;
    readSENData->massConcentrationPm2p5 = massConcentrationPm2p5;
    readSENData->massConcentrationPm4p0 = massConcentrationPm4p0;
    readSENData->massConcentrationPm10p0 = massConcentrationPm10p0;
    readSENData->ambientHumidity = ambientHumidity;
    readSENData->ambientTemperature = ambientTemperature;
    readSENData->vocIndex = vocIndex;
    readSENData->noxIndex = noxIndex;    

  if (error) {
    Serial.print("Error trying to execute readMeasuredValues(): ");
    errorToString(error, errorMessage, 256);
    Serial.println(errorMessage);
  }
}