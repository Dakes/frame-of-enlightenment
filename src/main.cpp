#include <Arduino.h>
#include <HTTPClient.h>
#include "WiFi.h"
#include "wanikani.h"
#include "utils.h"
#include "led.h"
#include "matrix.h"
#include "local_api.h"
#include "runtime_config.h"
#include "config.h"
#include "presence.h"
#include <FastLED.h>

// configure WiFi etc. in config.h

// global
int16_t lastReviews = -1;
int16_t lastLessons = -1;
WaniKani wk(API_KEY);
Led led = Led();
Matrix matrix = Matrix(&wk);
PresenceDetector presence(25, 34);
bool g_active = false;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.printf("Reset reason: %d\n", (int)esp_reset_reason());
    delay(500);
    Serial.println("Welcome to the Frame of Enlightenment");
    g_config.load();
    Utils::WifiConnect();
    randomSeed(millis());
    Serial.println((String)"FreeHeap: " + ESP.getFreeHeap()/1024);

    sleep(1);
    led.begin();

    presence.init();

    setupLocalApi(&wk);

    matrix.init();
    Serial.println("MILLIS_PER_FRAME: " + String(MILLIS_PER_FRAME));
    Serial.println("SPAWN_DELAY: " + String(SPAWN_DELAY));
}


void showWifi()
{
    static String lastSSID;
    if (WiFi.SSID() != lastSSID)
    {
        lastSSID = WiFi.SSID();

        Serial.print("IP address: ");
        Serial.print(WiFi.localIP());
        Serial.print("  | SSID: ");
        Serial.println(WiFi.SSID());
        Serial.println("WiFi.status(): " + String(WiFi.status()));
    }
}

void loop()
{

    bool shouldRun = presence.shouldBeActive();

    if (shouldRun && !g_active)
    {
        Serial.println("Presence detected, waking system...");
        presence.onSystemWake();
        wk.lastRequestTime = 0;
        wk.refresh();
        matrix.init();
        g_active = true;
    }
    else if (!shouldRun && g_active)
    {
        Serial.println("No presence, sleeping...");
        FastLED.clear();
        FastLED.show();
        g_active = false;
    }

    if (shouldRun)
    {
        showWifi();

        if (Utils::WiFiConnected())
        {
            wk.refresh();
        }

        handleLocalApi();

        EVERY_N_MILLISECONDS(MILLIS_PER_FRAME)
        {
            matrix.simulationStep();
            matrix.updateReviewFutureRow();
            matrix.checkReviewLessonCounts();

            led.displayMatrix(matrix);
        }
    }
    else
    {
        handleLocalApi();
        delay(100);
    }
}

