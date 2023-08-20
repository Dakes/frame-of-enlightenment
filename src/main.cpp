#include <Arduino.h>
// #include <FastLED.h>
#include <time.h>
#include <HTTPClient.h>
#include "WiFi.h"


#include "wanikani.h"
#include "utils.h"
#include "led.h"

// configure WiFi etc. in config.h

// global
int16_t lastReviews = -1;
int16_t lastLessons = -1;
WaniKani wk(API_KEY);
Led led = Led();

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(9600);
    Serial.println("Welcome to the Frame of Enlightenment");
    // delay(200);
    // int result = myFunction(2, 3);
    Utils::WifiConnect();
    Serial.println((String)"FreeHeap: " + ESP.getFreeHeap()/1024);

    sleep(1);
}

/*
int freeMemory() {
  char top;
  return &top - reinterpret_cast<char*>(sbrk(0));
}
*/

void loop()
{
    Serial.println((String)"FreeHeap: " + ESP.getFreeHeap()/1024);
    Serial.print("IP address: ");
    Serial.print(WiFi.localIP());
    Serial.print("  | SSID: ");
    Serial.println(WiFi.SSID());
    // Serial.println("WiFi.status(): " + String(WiFi.status()));

    // if (WiFi.isConnected())
    if (Utils::WiFiConnected())
    {
        wk.refresh();
    }

    Serial.println("Reviews: "+(String)wk.getReviews());
    Serial.println("Lessons: "+(String)wk.getLessons());

    Serial.println("Reviews in 1 hour: " + (String)wk.getReviews(1));
    Serial.println("Reviews in 2 hours: " + (String)wk.getReviews(2));
    Serial.println("Reviews in 3 hours: " + (String)wk.getReviews(3));
    Serial.println("Reviews in 4 hours: " + (String)wk.getReviews(4));
    Serial.println("Reviews in 5 hours: " + (String)wk.getReviews(5));

    // Serial.println("Free memory: " + (String)freeMemory());

    Serial.println((String)"FreeHeap: " + ESP.getFreeHeap()/1024);

    Serial.println();
    led.lightLeds(wk.getReviews(), wk.getLessons());

    // sleep(10);
}

