#include "wanikani.h"
#include "utils.h"

WaniKani::WaniKani(const char* apiKey)
{
    this->apiKey = apiKey;
    // this->refresh();
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
        // StaticJsonDocument<64> filter;
        // filter["total_count"] = true;

        // DynamicJsonDocument json(jsonSize);

        DeserializationError error = deserializeJson(
            (*json), 
            Utils::httpGETRequest(apiUrl, "Authorization", String("Bearer ") + this->apiKey)
        );//, DeserializationOption::Filter(filter));

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

// on Error returns -1
// ! @deprecated
int16_t WaniKani::apiAssignmentsRequest(String apiParameter)
{
    int16_t count = -1;
    String apiUrl = this->WK_API_URL + (String)"assignments" + apiParameter;
    DynamicJsonDocument json(1024);
    this->apiRequest(apiUrl, &json);
    /*
    
    if (!json.isNull() )
        count = json["total_count"];*/

    return count;
}

DynamicJsonDocument* WaniKani::apiSummaryRequest()
{
    static DynamicJsonDocument cachedRequest(SUMMARY_JSON_SIZE);
    static ulong lastRequestTime = 0;

    if (this->canRequest(lastRequestTime) && Utils::WiFiConnected())
    {
        Serial.println("Caching Request Json");
        if (this->apiRequest(this->WK_API_URL + (String)"summary", &cachedRequest))
            lastRequestTime = millis();
    }
    return &cachedRequest;
}

/**
 * Compares 2 times (millis). Returns true, if a new request can be made.
*/
bool WaniKani::canRequest(ulong prev, ulong newTime)
{
    if (prev <= 0)
        return true;
    return (newTime-prev)/(1000*60) > this->MIN;
}

bool WaniKani::canRequest(ulong prev)
{
    return this->canRequest(prev, millis());
}

int16_t WaniKani::setReviews()
{
    int16_t reviews = this->getSummaryReviews();

    if (reviews >= 0)
    {
        this->reviews = reviews;
        Serial.println("Updated Reviews: " + (String)reviews);
    }
    
    return this->reviews;
}

int16_t WaniKani::setLessons()
{
    int16_t lessons = this->getSummaryLessons();
    if (lessons >= 0)
    {
        this->lessons = lessons;
        Serial.println("Updated Lessons: " + (String)lessons);
    }
    return this->lessons;
}

int16_t WaniKani::getSummaryLessons()
{
    if (!this->apiSummaryRequest()->isNull())
        return (*this->apiSummaryRequest())["data"]["lessons"][0]["subject_ids"].size();
    return -1;

}

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
    else
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