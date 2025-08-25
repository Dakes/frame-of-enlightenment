#include "presence.h"

#ifdef ENABLE_PRESENCE

PresenceDetector::PresenceDetector(uint8_t pir, uint8_t ldr)
    : pirPin(pir), ldrPin(ldr) {}

void PresenceDetector::init() {
    pinMode(pirPin, INPUT);
}

bool PresenceDetector::hasPresence() {
    return digitalRead(pirPin) == HIGH;
}

bool PresenceDetector::isDay() {
    uint16_t value = analogRead(ldrPin);
    return value > dayThreshold;
}

void PresenceDetector::onSystemWake() {
    lastPresenceTime = millis();
    lastPresence = true;
}

bool PresenceDetector::shouldBeActive() {
    if (hasPresence()) {
        lastPresenceTime = millis();
        lastPresence = true;
        return true;
    }

    if (lastPresence) {
        uint32_t timeout = isDay() ? dayTimeout : nightTimeout;
        if (millis() - lastPresenceTime < timeout) {
            return true;
        }
        lastPresence = false;
    }
    return false;
}

#endif
