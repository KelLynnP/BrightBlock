// #include "Sen55Handler.h"

// Sen55Handler::Sen55Data Sen55Handler::pullData(){
//   char errorMessage[256];
//   uint16_t error;

//   error = sen5x.readMeasuredValues(
//     massConcentrationPm1p0, massConcentrationPm2p5, massConcentrationPm4p0,
//     massConcentrationPm10p0, ambientHumidity, ambientTemperature, vocIndex,
//     noxIndex);

//   if (error) {
//     Serial.print("Error trying to execute readMeasuredValues(): ");
//     errorToString(error, errorMessage, 256);
//     Serial.println(errorMessage);
//   }
// }

// void Sen55Handler::setup(){
// // Sensirion Data Rhings The used commands use up to 48 bytes. On some Arduino's the default buffer space is not large enough
//     #define MAXBUF_REQUIREMENT 48

//     #if (defined(I2C_BUFFER_LENGTH) && (I2C_BUFFER_LENGTH >= MAXBUF_REQUIREMENT)) || (defined(BUFFER_LENGTH) && BUFFER_LENGTH >= MAXBUF_REQUIREMENT)
//     #define USE_PRODUCT_INFO
//     #endif

//     SensirionI2CSen5x sen5x;
// }