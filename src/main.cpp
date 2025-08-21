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

    EVERY_N_MILLISECONDS(MILLIS_PER_FRAME)
    {
        // Serial.println("Reviews: "+(String)wk.getReviews());
        // Serial.println("Lessons: "+(String)wk.getLessons());

        // Serial.println("Reviews in 1 hour: " + (String)wk.getReviews(1));
        // Serial.println("Reviews in 2 hours: " + (String)wk.getReviews(2));
        // Serial.println("Reviews in 3 hours: " + (String)wk.getReviews(3));
        // Serial.println("Reviews in 4 hours: " + (String)wk.getReviews(4));
        // Serial.println("Reviews in 5 hours: " + (String)wk.getReviews(5));


        matrix.simulationStep();
        matrix.updateReviewFutureRow();

        led.displayMatrix(matrix);
    }
}

