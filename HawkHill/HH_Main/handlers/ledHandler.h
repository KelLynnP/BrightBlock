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
        OFF,
        NONE
    };

    LED(LEDType ledType);

    static const int indicatorGreenPin = 25;
    static const int indicatorRedPin = 26;
    static const int indicatorBluePin = 27;
    static const int beaconPin = 12;  

    void toggleStateBeacon(BeaconPattern pattern = BeaconPattern::NONE);
    void setIndicator(IndicatorPattern pattern, float sensorValue = 0.0);
    void update(float sensorValue = 0.0);

    bool beaconOverride = false;

private: 
    LEDType ledType;
    int toggleState = 0;
    
    static const int beaconFrequency = 1000;    // Hz
    static const int beaconResolution = 8;      // 8-bit resolution (0-255)
    static const int indicatorFrequency = 1000; // Hz
    static const int indicatorResolution = 8;   // 8-bit resolution (0-255)

    unsigned long patternStartTime = 0;
    unsigned long currentStateTime = 0;
    size_t currentStateIndex = 0;

    // Define color reduction factors as static constexpr
    static constexpr float redReduction = 1.0;
    static constexpr float greenReduction = 0.2;
    static constexpr float blueReduction = 0.1;

    // Initialize the map with the constants
    std::map<IndicatorColor, float> colorReduction = {
        {IndicatorColor::RED, redReduction},
        {IndicatorColor::GREEN, greenReduction},
        {IndicatorColor::BLUE, blueReduction}
    };

    IndicatorPattern currentPattern = IndicatorPattern::OFF;

    std::map<BeaconPattern, int> beaconBrightnessMap = {
        {BeaconPattern::MINIMUM, 100},
        {BeaconPattern::MAXIMUM, 255},
        {BeaconPattern::OFF, 0}
    };

    std::map<IndicatorColor, int> indicatorPinMap{
        {IndicatorColor::RED, indicatorRedPin},
        {IndicatorColor::GREEN, indicatorGreenPin},
        {IndicatorColor::BLUE, indicatorBluePin}
    };

    struct PatternState {
        uint8_t RED, GREEN, BLUE;     // Brightness via PWM resolution  (0-255)
        uint32_t duration;    // State duration in ms
    };

    using LEDPattern = std::vector<PatternState>;

    // Patterns: Consider pulling low => 0 is on!
    const std::map<IndicatorPattern, LEDPattern> staticPatterns = {  
        {IndicatorPattern::STARTUP, {
            {0, 255, 255, 1000},    // Red for 1s
            {255, 0, 255, 5000},    // Green for 1s
        }},
        
        {IndicatorPattern::LOW_BATTERY, {
            {0, 255, 255, 500},     // Red flash for 500ms
            {255, 255, 255, 2000}   // Off for 2s
        }},

        {IndicatorPattern::OFF, {
            {255, 255, 255, 0}      // All off
        }}
    };

    const char* colorToString(IndicatorColor color) {
        switch (color) {
            case IndicatorColor::RED: return "RED";
            case IndicatorColor::GREEN: return "GREEN";
            case IndicatorColor::BLUE: return "BLUE";
            default: return "UNKNOWN";
        }
    }
};

#endif
