#ifndef PRESENCE_H
#define PRESENCE_H

#include <Arduino.h>

#ifdef ENABLE_PRESENCE
class PresenceDetector {
private:
    uint8_t pirPin;
    uint8_t ldrPin;
    const uint16_t dayThreshold = 2048; // ADC midpoint
    const uint32_t dayTimeout = 300000; // 5 minutes
    const uint32_t nightTimeout = 0; // immediate
    bool lastPresence = false;
    uint32_t lastPresenceTime = 0;
public:
    PresenceDetector(uint8_t pir, uint8_t ldr);
    void init();
    bool shouldBeActive();
    void onSystemWake();
    bool isDay();
    bool hasPresence();
};
#else
class PresenceDetector {
public:
    PresenceDetector(uint8_t, uint8_t) {}
    void init() {}
    bool shouldBeActive() { return true; }
    void onSystemWake() {}
    bool isDay() { return true; }
    bool hasPresence() { return true; }
};
#endif

#endif
