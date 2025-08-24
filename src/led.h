#ifndef LED_h
#define LED_h

#include <Arduino.h>
#include <FastLED.h>
#include "utils.h"
#include "runtime_config.h"

#define HUE_LESSON (g_config.hueLesson)          // pink-ish 320°: 227
#define HUE_REVIEW (g_config.hueReview)          // cyan-ish 200°: 142
#define HUE_REVIEW_FUTURE (g_config.hueReviewFuture)    // orange


// See: https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino for details about the options
// Physical matrix size in pixels
#define MATRIX_WIDTH  10  // adjust
#define MATRIX_HEIGHT 10  // adjust

// factor by which the simulation matrix is higher resolution than the LED matrix
#define MATRIX_RESOLUTION 10

// scale derived from resolution for timing adjustments (half resolution, min 1)
#define MATRIX_RESOLUTION_SCALE ((MATRIX_RESOLUTION + 1) / 2)

// internal high-resolution matrix dimensions
#define MATRIX_INTERNAL_WIDTH  (MATRIX_WIDTH  * MATRIX_RESOLUTION)
#define MATRIX_INTERNAL_HEIGHT (MATRIX_HEIGHT * MATRIX_RESOLUTION)

class Matrix; // forward declaration

class Led {

private:
    #define DATA_PIN 27
    #define LED_TYPE WS2811
    #define COLOR_ORDER GRB

    // HSV Saturation
    #define S 255
    // HSV value (brightness / Luma)
    #define V 255

    #define MIN_S (g_config.minS)
    #define MIN_V (g_config.minV)

    #define FRAMERATE (g_config.frameRate)
    #define MILLIS_PER_FRAME (1000 / FRAMERATE)
    // Animation delay in ms
    #define ANIM_DELAY 10
    // Animation delay for hourglass fill
    #define SPAWN_DELAY (50 / MATRIX_RESOLUTION_SCALE)


    #define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
    #define LAST_VISIBLE_LED (NUM_LEDS - 1)
    // at how many Reviews+Lessons the frame should fully light up
    #define FRAME_FULL (g_config.frameFull)
    #define ITEMS_PER_PIXEL (FRAME_FULL / (MATRIX_WIDTH * MATRIX_HEIGHT))

    const bool kMatrixSerpentineLayout = true;
    const bool kMatrixVertical = false;

    void setup();
    void printLeds();
    void fadeToHue(CRGB* pixel, uint8_t targetHue);
    void fadeOn(CRGB* pixel, uint8_t hue);
    void fadeOff(CRGB* pixel);
    void show();
    void show(u16_t delay);

    struct lightLedParameters
    {
        CRGB* led;
        int16_t* count;
        uint8_t hue;
        Led* self;
    };

public:
    Led();
    void begin();
    void loop();
    void lightLeds(int16_t reviews, int16_t lessons);
    void animateUpcomingReviews(int16_t reviews1h, int16_t reviews2h, int16_t reviews3h, int16_t reviews4h, int16_t reviews5h);
    void clearAll();
    uint16_t XY(uint8_t x, uint8_t y);
    CRGB leds[NUM_LEDS]; // LED array

    // Helper methods for direct LED manipulation
    void setPixelColor(uint8_t x, uint8_t y, uint8_t r, uint8_t g, uint8_t b);
    void showLeds();
    void testPattern();
    void displayMatrix(const Matrix& matrix);

};

#endif
