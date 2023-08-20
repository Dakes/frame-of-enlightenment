#ifndef LED_h
#define LED_h

#include <Arduino.h>
#include <FastLED.h>
#include "utils.h"

class Led {

private:
    #define DATA_PIN 27
    #define LED_TYPE WS2811
    #define COLOR_ORDER GRB

    // HSV Saturation
    #define S 255
    // HSV value (brightness / Luma)
    #define V 255
    // Animation delay in ms
    #define ANIM_DELAY 10


    // See: https://github.com/FastLED/FastLED/blob/master/examples/XYMatrix/XYMatrix.ino for details about the options
    #define MATRIX_WIDTH  10  // adjust
    #define MATRIX_HEIGHT 10  // adjust

    #define NUM_LEDS (MATRIX_WIDTH * MATRIX_HEIGHT)
    CRGB leds[NUM_LEDS];
    #define LAST_VISIBLE_LED (NUM_LEDS - 1)
    const bool kMatrixSerpentineLayout = true;
    const bool kMatrixVertical = false;

    #define HUE_LESSON 220  // pink-ish 320°: 227
    #define HUE_REVIEW 142  // cyan-ish 200°: 142
    const uint8_t hueRandomness = 15;  // add a random value between +- this to the hue, for some variation

    // at how many Reviews+Lessons the frame should fully light up
    const uint16_t frameFull = 1000;
    // Whether to fill the frame logarithmically or linearly
    // Use log scaling to make the frame more useful at low Review counts.
    // actually scales linearly (1 Review = 1 LED) until intersection
    bool logScaling = true;  // changeable
    #define LOG_BASE (exp(log(frameFull)/frameFull))
    #define CONSTANT_SCALING_FACTOR (frameFull/(pow(frameFull, 0.5)))

    uint8_t* genXIterOrder()
    {
        uint8_t* arr = (uint8_t*) malloc(MATRIX_WIDTH * sizeof(uint8_t));
        int i=0;
        for (int m=0; m <= MATRIX_WIDTH/2; m++)
        {
            arr[i] = (MATRIX_WIDTH/2)-m;
            i++;
            if (m != 0 && (float)m != (float)MATRIX_WIDTH/2)
            {
                arr[i] = (MATRIX_WIDTH/2)+m;
                i++;
            }
        }
        return arr;
    }
    const uint8_t* xIterOrder = genXIterOrder();

    
    uint16_t XY(uint8_t x, uint8_t y);
    

    void setup();
    uint16_t scaleToLeds(uint16_t num);
    uint16_t scaleToLeds(uint16_t num, bool linear);
    void lightLessons(uint16_t lessons);
    void lightReviews(uint16_t reviews);
    void printLeds();
    uint8_t randomizeHue(uint8_t hue);
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
    void lightLed(CRGB* led, int16_t* count, const uint8_t hue, uint8_t core=0);
    static void lightLedMT(void* parameters);
    // bool setLed(int16_t* reviews, int16_t* lessons, uint8_t x, uint8_t y);

public:
    Led();
    void loop();
    void lightLeds(int16_t reviews, int16_t lessons);
    
};

#endif