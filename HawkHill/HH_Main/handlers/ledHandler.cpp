#include "ledHandler.h"


LED::LED(LEDType ledType) 
    : ledType(ledType) {
    patternStartTime = millis();
    currentStateIndex = 0;
    if (ledType == LEDType::INDICATOR) {
        for (const auto& [color, pin] : indicatorPinMap) {
            if (ledcAttach(pin, indicatorFrequency, indicatorResolution)) {
                Serial.println("LED: " + String(colorToString(color)) + " attached to pin: " + String(pin));
            } else {
                Serial.println("LED: " + String(colorToString(color)) + " failed to attach to pin: " + String(pin));
            }
        }
    } else if (ledType == LEDType::BEACON) {
        ledcAttach(LED::beaconPin, beaconFrequency, beaconResolution);
        toggleStateBeacon(BeaconPattern::OFF);
        Serial.println("Beacon attached");
    }
}

void LED::toggleStateBeacon(BeaconPattern pattern) {
    // If a pattern is requested, set the beacon to that pattern
    if (pattern != BeaconPattern::NONE) {
        ledcWrite(beaconPin, beaconBrightnessMap[pattern]);
        Serial.println("Beacon set to: " + String(static_cast<int>(pattern)));
        return;
    }

    // If no pattern is requested, toggle state
    toggleState = (toggleState + 1) % 3;
    if (beaconOverride) {
        toggleState = 0;
        Serial.println("Beacon light in safety override mode");
    }
    
    switch (toggleState) {
        case 0: 
            ledcWrite(beaconPin, beaconBrightnessMap[BeaconPattern::OFF]);
            Serial.println("Beacon OFF"); 
            break;
        case 1: 
            ledcWrite(beaconPin, beaconBrightnessMap[BeaconPattern::MINIMUM]);
            Serial.println("Beacon LOW"); 
            break;
        case 2: 
            ledcWrite(beaconPin, beaconBrightnessMap[BeaconPattern::MAXIMUM]);
            Serial.println("Beacon HIGH"); 
            break;
    }

}


void LED::setIndicator(IndicatorPattern pattern, float sensorValue) {
    currentPattern = pattern;
    if (pattern != IndicatorPattern::FOLLOW_POLLUTION) {
        // Get the pattern sequence for the requested pattern
        const auto& patternSequence = staticPatterns.at(pattern);
        
        // Get the current state from the pattern sequence
        const auto& currentState = patternSequence[currentStateIndex];
        
        // Map each color to its corresponding pin and brightness
        std::map<IndicatorColor, uint8_t> colorBrightness = {
            {IndicatorColor::RED, currentState.RED},
            {IndicatorColor::GREEN, currentState.GREEN},
            {IndicatorColor::BLUE, currentState.BLUE}
        };

        // Apply the brightness values to each LED
        for (const auto& [color, pin] : indicatorPinMap) {
            uint8_t brightness = colorBrightness[color] * colorReduction[color];
            ledcWrite(pin, brightness);
            // Serial.println("LED " + String(colorToString(color)) + 
            //              " set to brightness: " + String(brightness));
        }

        // Update timing
        if (millis() - patternStartTime >= currentState.duration) {
            currentStateIndex = (currentStateIndex + 1) % patternSequence.size();
            patternStartTime = millis();
            Serial.println("Moving to pattern state: " + String(currentStateIndex));
        }
    }
    else {
        // Follow pollution pattern
        struct RGBPoint {
            int index;
            uint8_t r, g, b;
        };

        // Define the color points
        const std::vector<RGBPoint> colorPoints = {
            {0,   0, 255, 0},   // Healthy green
            {400, 255, 0,  0}    // Dark Purple
        };

        // const std::vector<RGBPoint> colorPoints = {
        //     {0,   116, 158, 71},   // Healthy green
        //     {75,  242, 194, 79},   // Yellow
        //     {125, 229, 131, 49},   // Orange
        //     {175, 182, 68,  51},   // Red-Orange
        //     {250, 159, 39,  87},   // Purple-Red
        //     {400, 124, 29,  78}    // Dark Purple
        // };

        // Find the two points to interpolate between
        size_t i = 0;
        while (i < colorPoints.size() - 1 && sensorValue > colorPoints[i + 1].index) {
            i++;
        }

        // If we're at or beyond the last point, use the last color
        if (i >= colorPoints.size() - 1) {
            ledcWrite(indicatorPinMap[IndicatorColor::RED], colorPoints.back().r * colorReduction[IndicatorColor::RED]);
            ledcWrite(indicatorPinMap[IndicatorColor::GREEN], colorPoints.back().g * colorReduction[IndicatorColor::GREEN]);
            ledcWrite(indicatorPinMap[IndicatorColor::BLUE], colorPoints.back().b * colorReduction[IndicatorColor::BLUE]);
            return;
        }

        // Calculate interpolation factor
        const auto& p1 = colorPoints[i];
        const auto& p2 = colorPoints[i + 1];
        float t = (sensorValue - p1.index) / (p2.index - p1.index);

        // Interpolate RGB values
        uint8_t r = p1.r + t * (p2.r - p1.r);
        uint8_t g = p1.g + t * (p2.g - p1.g);
        uint8_t b = p1.b + t * (p2.b - p1.b);

        // Set LED colors
        ledcWrite(indicatorPinMap[IndicatorColor::RED], 255 - r * colorReduction[IndicatorColor::RED]);
        ledcWrite(indicatorPinMap[IndicatorColor::GREEN], 255 - g * colorReduction[IndicatorColor::GREEN]);
        ledcWrite(indicatorPinMap[IndicatorColor::BLUE], 255 - b * colorReduction[IndicatorColor::BLUE]);

        // Serial.println("Pollution LED - R:" + String(r) + 
        //               " G:" + String(g) + 
        //               " B:" + String(b) + 
        //               " (value: " + String(sensorValue) + ")");
    }
}


void LED::update(float sensorValue) {
    if (ledType == LEDType::INDICATOR) {
        setIndicator(currentPattern, sensorValue);
    }
}
