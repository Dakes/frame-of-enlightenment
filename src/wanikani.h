#ifndef WANIKANI_h
#define WANIKANI_h

#include <Arduino.h>
#include <ArduinoJson.h>
#include <HTTPClient.h>

#include "utils.h"

class WaniKani {
private:
    // minimum time between requests to API in minutes
    const uint8_t MIN = 0;
    static const uint16_t SUMMARY_JSON_SIZE = 32768;

    int16_t reviews = -1;
    int16_t lessons = -1;
    const char* apiKey;
    const char* WK_API_URL = "https://api.wanikani.com/v2/";
    bool apiRequest(String apiUrl, DynamicJsonDocument* json);
    int16_t apiAssignmentsRequest(String apiParameter);
    DynamicJsonDocument* apiSummaryRequest();
    int16_t getSummaryLessons();
    int16_t getSummaryReviews(uint8_t hour=0);
    int16_t setReviews();
    int16_t setLessons();
    bool canRequest(ulong prev, ulong newTime);
    bool canRequest(ulong prev);

public:
    WaniKani(const char* apiKey);
    int16_t getReviews();
    // get upcoming Reviews that many hours in the future
    int16_t getReviews(uint8_t hour);
    int16_t getLessons();
    void refresh();
};

#endif