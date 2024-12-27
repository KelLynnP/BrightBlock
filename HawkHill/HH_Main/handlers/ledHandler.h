#ifndef LED_H
#define LED_H

#include <Arduino.h>
#include <map>
#include <vector>

class LED {

public:
enum class LEDType {
    BEACON,
    INDICATOR
}; 

enum class IndicatorColor {
    RED,
    GREEN,
    BLUE
};

enum class IndicatorPattern {
    FOLLOW_POLLUTION,
    LOW_BATTERY,
    STARTUP, 
    OFF
};

enum class BeaconPattern {
    MINIMUM,
    MAXIMUM,
    OFF
};

LED(LEDType ledType);

static const int indicatorGreenPin = 25;
static const int indicatorRedPin = 26;
static const int indicatorBluePin = 27;

void toggleBeacon();
void setBeacon(BeaconPattern pattern = BeaconPattern::OFF);
void setIndicator(IndicatorPattern pattern);
void update(float sensorValue = 0.0);  // Add sensor value parameter to update

std::map<IndicatorColor, int> indicatorPinMap{
    {LEDType::RED_INDICATOR, indicatorRedPin},
    {LEDType::GREEN_INDICATOR, indicatorGreenPin},
    {LEDType::BLUE_INDICATOR, indicatorBluePin}
};

private: 
    const int pin;
    const int channel;

    static const int beaconFrequency = 1000;    // 1 KHz
    static const int beaconResolution = 8;      // 8-bit resolution (0-255)

    static const int indicatorFrequency = 1000;    // 1 KHz
    static const int indicatorResolution = 8;      // 8-bit resolution (0-255)

    unsigned long patternStartTime = 0;
    unsigned long currentStateTime = 0;
    size_t currentStateIndex = 0;

    IndicatorPattern currentPattern = IndicatorPattern::OFF;

    std::map<BeaconPattern, int> beaconBrightnessMap = {
        {BeaconPattern::MINIMUM, 100},
        {BeaconPattern::MAXIMUM, 255},
        {BeaconPattern::OFF, 0}
    };

    struct PatternState {
        uint8_t r, g, b;     // Brightness via PWM resolution  (0-255)
        uint32_t duration;    // State duration in ms
    };

    using LEDPattern = std::vector<PatternState>;

    // Pattern definitions
    const std::map<IndicatorPattern, LEDPattern> staticPatterns = {
        {IndicatorPattern::STARTUP, {
            {255, 0, 0, 1000},    // Red for 1s
            {0, 255, 0, 1000},    // Green for 1s
            {0, 0, 255, 1000}     // Blue for 1s
        }},
        
        {IndicatorPattern::LOW_BATTERY, {
            {255, 0, 0, 500},     // Red flash for 500ms
            {0, 0, 0, 2000}       // Off for 2s
        }},

        {IndicatorPattern::OFF, {
            {0, 0, 0, 0}          // All off
        }}
    };
};

#endif
