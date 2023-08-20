#ifndef UTILS_h
#define UTILS_h

#include <Arduino.h>
#include <HTTPClient.h>
#include <WiFi.h>
#include "config.h"

namespace Utils {

    void WifiConnect();
    bool WiFiConnected();
    String httpGETRequest(const String serverName, const String headerName="", const String headerValue="");
    uint32_t pair(int16_t a, int16_t b);

};

#endif