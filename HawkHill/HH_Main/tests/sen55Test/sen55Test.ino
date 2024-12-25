#include "../../handlers/sen55Handler.h"
#include "../../handlers/sen55Handler.cpp"

// Define custom I2C pins
static const int CUSTOM_SDA = 23;
static const int CUSTOM_SCL = 22;

Sen55Handler sen55;

void setup() {
    Serial.begin(115200);
    
    // Print default I2C pins
    Serial.println("Default I2C Pins:");
    Serial.printf("Default SDA: %d\n", SDA);
    Serial.printf("Default SCL: %d\n", SCL);
    
    // Print custom I2C pins
    Serial.println("\nUsing Custom I2C Pins:");
    Serial.printf("Custom SDA: %d\n", CUSTOM_SDA);
    Serial.printf("Custom SCL: %d\n", CUSTOM_SCL);
    
    // Initialize I2C with custom pins
    Wire.begin(CUSTOM_SDA, CUSTOM_SCL);
    
    sen55.setup();
    Serial.println("SEN55 Setup Complete");
}

void loop() {
    if (sen55.pullData()) {
        Serial.println("\nNew Reading:");
        Serial.printf("PM2.5: %.2f µg/m³\n", sen55.getPm2p5());
        Serial.printf("Temperature: %.2f °C\n", sen55.getAmbientTemperature());
        Serial.printf("Humidity: %.2f %%RH\n", sen55.getAmbientHumidity());
        Serial.printf("VOC Index: %.2f\n", sen55.getVocIndex());
        Serial.printf("NOx Index: %.2f\n", sen55.getNoxIndex());
    }
    
    delay(1000);
}
