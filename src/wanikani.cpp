#include "wanikani.h"
#include "utils.h"

WaniKani::WaniKani(const char* apiKey) : summaryCache(SUMMARY_JSON_SIZE)
{
    this->apiKey = apiKey;
}

/**
 * @param json: reference to json to update
 * @return: bool: success
*/
bool WaniKani::apiRequest(String apiUrl, DynamicJsonDocument* json)
{
    bool success = false;
    if (Utils::WiFiConnected())
    {
        DeserializationError error = deserializeJson(
            (*json),
            Utils::httpGETRequest(apiUrl, "Authorization", String("Bearer ") + this->apiKey)
        );

        if (error)
        {
            Serial.print(F("WaniKani::apiRequest deserializeJson() failed: "));
            Serial.println(error.f_str());
            Serial.println("apiUrl: " + apiUrl);
        }
        else
            success = true;
    }
    return success;
}

DynamicJsonDocument* WaniKani::apiSummaryRequest()
{
    if (overrideSummary)
        return &summaryCache;

    if (this->canRequest(lastRequestTime) && Utils::WiFiConnected())
    {
        Serial.println("Caching Request Json");
        if (this->apiRequest(this->WK_API_URL + (String)"summary", &summaryCache))
            lastRequestTime = millis();
    }
    return &summaryCache;
}

bool WaniKani::setSummaryJson(const String& json)
{
    DeserializationError error = deserializeJson(summaryCache, json);
    if (error)
    {
        Serial.print(F("WaniKani::setSummaryJson deserializeJson() failed: "));
        Serial.println(error.f_str());
        return false;
    }
    lastRequestTime = millis();
    overrideSummary = true;
    return true;
}

/**
 * Compares 2 times (millis). Returns true, if a new request can be made.
*/
bool WaniKani::canRequest(ulong prev, ulong newTime)
{
    if (prev <= 0)
        return true;
    return (newTime - prev) / (1000 * 60) > this->MIN;
}

bool WaniKani::canRequest(ulong prev)
{
    return this->canRequest(prev, millis());
}

int16_t WaniKani::setReviews()
{
    int16_t reviews = this->getSummaryReviews();
    static int16_t lastReviews = -1;

    if (reviews >= 0)
    {
        this->reviews = reviews;
        if (lastReviews != reviews)
        {
            Serial.println("Updated Reviews: " + (String)reviews);
            lastReviews = reviews;
        }
    }

    return this->reviews;
}

int16_t WaniKani::setLessons()
{
    int16_t lessons = this->getSummaryLessons();
    static int16_t lastLessons = -1;

    if (lessons >= 0)
    {
        this->lessons = lessons;
        if (lastLessons != lessons)
        {
            Serial.println("Updated Lessons: " + (String)lessons);
            lastLessons = lessons;
        }
    }
    return this->lessons;
}

int16_t WaniKani::getSummaryLessons()
{
    if (!this->apiSummaryRequest()->isNull())
        return (*this->apiSummaryRequest())["data"]["lessons"][0]["subject_ids"].size();
    return -1;
}

/**
 *
 * @param hour represents how many hours in the future the reviews appear. can be up to 24
 * @return number of reviews at that hour
 */
int16_t WaniKani::getSummaryReviews(uint8_t hour)
{
    if (!this->apiSummaryRequest()->isNull())
        return (*this->apiSummaryRequest())["data"]["reviews"][hour]["subject_ids"].size();
    return -1;
}

int16_t WaniKani::getReviews()
{
    return this->reviews;
}


int16_t WaniKani::getReviews(uint8_t hour)
{
    if (hour == 0)
        return this->reviews;
    return this->getSummaryReviews(hour);
}

int16_t WaniKani::getLessons()
{
    return this->lessons;
}

void WaniKani::refresh()
{
    this->setReviews();
    this->setLessons();
}
