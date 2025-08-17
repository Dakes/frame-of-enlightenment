#include <Arduino.h>
// #include <FastLED.h>
#include <time.h>
#include <HTTPClient.h>
#include "WiFi.h"


#include "wanikani.h"
#include "utils.h"
#include "led.h"
#include "matrix.h"

// configure WiFi etc. in config.h

// global
int16_t lastReviews = -1;
int16_t lastLessons = -1;
WaniKani wk(API_KEY);
Led led = Led();
Matrix matrix;

void setup()
{
    // put your setup code here, to run once:
    Serial.begin(115200);
    Serial.printf("Reset reason: %d\n", (int)esp_reset_reason());
    delay(500);
    Serial.println("Welcome to the Frame of Enlightenment");
    // delay(200);
    // int result = myFunction(2, 3);
    Utils::WifiConnect();
    Serial.println((String)"FreeHeap: " + ESP.getFreeHeap()/1024);

    sleep(1);
    led.begin();
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

    matrix.clear();
    int idx = 0;
    auto place = [&](CellType type, uint32_t availableAt, int count)
    {
        for (int i = 0; i < count && idx < MATRIX_WIDTH * MATRIX_HEIGHT; ++i, ++idx)
        {
            uint8_t x = idx % MATRIX_WIDTH;
            uint8_t y = idx / MATRIX_WIDTH;
            matrix.setCell(x, y, type, availableAt);
        }
    };
    place(CELL_LESSON, 0, wk.getLessons());
    place(CELL_REVIEW, 0, wk.getReviews());
    place(CELL_REVIEW, 1, wk.getReviews(1));
    led.displayMatrix(matrix);

    // sleep(10);
}

