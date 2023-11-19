#include "LPS2.h"

void LPS2::setup() {

  if (!lps.begin_SPI(LPS_CS)) {
    Serial.println("Failed to find LPS22 chip");
    while (1) { delay(10); }
  }

  Serial.println("LPS22 Found!");
  lps.setDataRate(LPS22_RATE_10_HZ);
  Serial.print("Data rate set to: ");
  switch (lps.getDataRate()) {
    case LPS22_RATE_ONE_SHOT: Serial.println("One Shot / Power Down"); break;
    case LPS22_RATE_1_HZ: Serial.println("1 Hz"); break;
    case LPS22_RATE_10_HZ: Serial.println("10 Hz"); break;
    case LPS22_RATE_25_HZ: Serial.println("25 Hz"); break;
    case LPS22_RATE_50_HZ: Serial.println("50 Hz"); break;
    case LPS22_RATE_75_HZ: Serial.println("75 Hz"); break;
  }
}

void LPS2::pullData() {
  sensors_event_t pressureSE;
  sensors_event_t tempSE;

  lps.getEvent(&pressureSE, &tempSE); 
  pressure = pressureSE.pressure;
  temp = tempSE.temperature;
}

