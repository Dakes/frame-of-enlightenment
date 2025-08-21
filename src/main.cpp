#include <Arduino.h>
// #include <FastLED.h>
#include <time.h>
#include <HTTPClient.h>
#include "WiFi.h"
#include "wanikani.h"
#include "utils.h"
#include "led.h"
#include "matrix.h"
#include "local_api.h"

// configure WiFi etc. in config.h

// global
int16_t lastReviews = -1;
int16_t lastLessons = -1;
WaniKani wk(API_KEY);
Led led = Led();
Matrix matrix = Matrix(&wk);

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

    setupLocalApi(&wk);


    matrix.clear();

    // test sets
    // matrix.setCell(Coord(0, 0), CELL_REVIEW, V);
    // matrix.setCell(Coord(1, 1), CELL_REVIEW, V);
    // matrix.setCell(Coord(2, 5), CELL_REVIEW, V);
    // matrix.setCell(Coord(3, 6), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 9), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 8), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 7), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 6), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 5), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 4), CELL_LESSON, V);
    // matrix.setCell(Coord(3, 3), CELL_LESSON, V);
    //
    // matrix.setCell(Coord(2, 9),CELL_REVIEW ,V);
    // matrix.setCell(Coord(2, 8),CELL_REVIEW ,V);
    // matrix.setCell(Coord(2, 7),CELL_REVIEW ,V);
    // matrix.setCell(Coord(2, 6),CELL_REVIEW ,V);
    // matrix.setCell(Coord(2, 5),CELL_REVIEW ,V);
    // matrix.setCell(Coord(2, 4),CELL_REVIEW ,V);
    // matrix.setCell(Coord(2, 3),CELL_REVIEW ,V);
    //
    // matrix.setCell(Coord(1, 9), CELL_LESSON, V);
    // matrix.setCell(Coord(1, 8), CELL_LESSON, V);
    // matrix.setCell(Coord(1, 7), CELL_LESSON, V);
    // matrix.setCell(Coord(1, 6), CELL_LESSON, V);
    // matrix.setCell(Coord(1, 5), CELL_LESSON, V);
    // matrix.setCell(Coord(1, 4), CELL_LESSON, V);
    // matrix.setCell(Coord(1, 3), CELL_LESSON, V);
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

    showWifi();

    if (Utils::WiFiConnected())
    {
        wk.refresh();
    }

    handleLocalApi();

    EVERY_N_MILLISECONDS(MILLIS_PER_FRAME)
    {
        // Serial.println("Reviews: "+(String)wk.getReviews());
        // Serial.println("Lessons: "+(String)wk.getLessons());



        matrix.checkReviewLessonCounts();
        matrix.simulationStep();
        matrix.updateReviewFutureRow();

        led.displayMatrix(matrix);
    }
}

