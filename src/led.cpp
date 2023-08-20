#include "led.h"

Led::Led()
{
    this->setup();
}

void Led::setup()
{
    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.clear();
}

uint16_t Led::XY(uint8_t x, uint8_t y)
{
    uint16_t i;

    if (kMatrixSerpentineLayout == false)
    {
        if (kMatrixVertical == false)
            i = (y * MATRIX_WIDTH) + x;
        else
            i = MATRIX_HEIGHT * (MATRIX_WIDTH - (x + 1)) + y;
    }

    if (kMatrixSerpentineLayout == true)
    {
        if (kMatrixVertical == false)
        {
            if (y & 0x01)
            {
                // Odd rows run backwards
                uint8_t reverseX = (MATRIX_WIDTH - 1) - x;
                i = (y * MATRIX_WIDTH) + reverseX;
            }
            else  // Even rows run forwards
                i = (y * MATRIX_WIDTH) + x;
        }
        else
        { // vertical positioning
            if (x & 0x01)
                i = MATRIX_HEIGHT * (MATRIX_WIDTH - (x + 1)) + y;
            else
                i = MATRIX_HEIGHT * (MATRIX_WIDTH - x) - (y + 1);
        }
    }

    return i;
}

void Led::loop()
{
    for( uint8_t x = 0; x < MATRIX_WIDTH; x++)
    {
        for( uint8_t y = 0; y < MATRIX_HEIGHT; y++)
        {
            // Here's the x, y to 'led index' in action: 
            leds[ XY(x, y) ].setHSV(random8(), 255, 255);

            FastLED.show();
            delay(100);
        }
    }
    delay(500);
    FastLED.clear();

}

/**
 * Logarithmically or linearly scale num to number of LEDs
 * But doesn't actually scale logarithmically, but somewhere between log and linearly
*/
uint16_t Led::scaleToLeds(uint16_t num)
{
    return scaleToLeds(num, !logScaling);
}

uint16_t Led::scaleToLeds(uint16_t num, bool linear)
{
    if (num > frameFull)
        num = frameFull;
    if (logScaling)
    {
        uint16_t scaled = (int)(((pow(num, 0.5)*CONSTANT_SCALING_FACTOR)/(float)frameFull)*NUM_LEDS);
        if (scaled > num)
            return num;
        return scaled;
    }
    else // linear
    {
        return (int)((num/(float)frameFull) * NUM_LEDS);
    }
}

/**
 * @param lessons number of lessons
*/
void Led::lightLessons(uint16_t lessons)
{
    uint16_t ledsToLight = scaleToLeds(lessons);
}

void Led::printLeds()
{
    Serial.println("Led matrix: ");
    for( uint8_t y = MATRIX_HEIGHT; y > 0; y--)
    {
        for( uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            int led = leds[ XY(x, y) ][0]+leds[ XY(x, y) ][1]+leds[ XY(x, y) ][2]+leds[ XY(x, y) ][3];
            if (led<10) Serial.print(" ");
            if (led<100) Serial.print(" ");
            Serial.print(led);
            Serial.print("|");
        }
        Serial.println();
    }
    Serial.println();
}

/**
 * turn on individual LED, first lessons, then reviews. 
 * Reduces passed pointer int values. 
 * @return true: default. false, if nothing can be done any more
*/ /*
bool Led::setLed(int16_t* reviews, int16_t* lessons, uint8_t x, uint8_t y)
{
    uint8_t hue = HUE_REVIEW;
    if (lessons > 0)
        hue = HUE_LESSON;
    else if (reviews > 0)
        hue = HUE_REVIEW;
    else
        return false;
    
}*/

uint8_t Led::randomizeHue(uint8_t hue)
{
    return hue + random(-hueRandomness, hueRandomness+1);
}

/**
 * Fade to targetHue smoothly. 
 * pixel is a pointer to the individual pixel from Fastled led matrix.
 * If pixel is off, fades it to on at that hue from the beginning.
*/
void Led::fadeToHue(CRGB* pixel, uint8_t targetHue)
{
    if(pixel->getLuma() == 0)
        fadeOn(pixel, targetHue);
    else
    {
        CHSV hsvPix(rgb2hsv_approximate(*pixel));
        while(hsvPix.hue != targetHue)
        {
            if (hsvPix.hue < targetHue)
                hsvPix.hue ++;
            else
                hsvPix.hue --;
            pixel->setHue(hsvPix.hue);
            show(ANIM_DELAY*3);
        }
    }
}


void Led::fadeOff(CRGB* pixel)
{
    const uint8_t fading = 10;//pixel->getLuma();
    uint8_t prevLuma = 0;
    while(pixel->getLuma() > 0)
    {
        if (prevLuma == pixel->getLuma())
        {
            pixel->setRGB(0, 0, 0);
            show();
            return;
        }

        prevLuma = pixel->getLuma();
        // Serial.println("fadeOff: " + String(pixel->getLuma()));
        // fading--;
        pixel->fadeToBlackBy(fading);
        show();
    }

}

void Led::fadeOn(CRGB* pixel, u8_t hue)
{
    if (pixel->getAverageLight() > V/4)
        return;
    pixel->setHue(hue);
    
    for(int v=0; v<V; v+=20)
    {
        if (v>V) v=V;
        pixel->setHSV(hue, S, v);
        show(ANIM_DELAY/2);
    }
    pixel->setHSV(hue, S, V);
}

/**
 * Show with delay
*/
void Led::show()
{
    show(ANIM_DELAY);
}

void Led::show(u16_t delay)
{
    FastLED.delay(delay);
    FastLED.show();
}


/**
 * turn on this led at hue and reduce passed led count "count"
*/
void Led::lightLed(CRGB* led, int16_t* count, const uint8_t hue, uint8_t core)
{
    struct lightLedParameters* params = (struct lightLedParameters*) malloc(sizeof(struct lightLedParameters));
    params->led = led;
    params->count = count;
    params->hue = hue;
    params->self = this;

    TaskHandle_t Task1;

    xTaskCreatePinnedToCore(
        lightLedMT,   /* Function to implement the task */
        "LightLed", /* Name of the task */
        1000,       /* Stack size in words */
        params,     /* Task input parameter */
        0,          /* Priority of the task */
        &Task1,     /* Task handle. */
        core);      /* Core where the task should run */


    // lightLed(params);
}

void Led::lightLedMT(void* parameters)
{
    lightLedParameters* data = (lightLedParameters*) parameters;
    if (data->count)
    {
        data->self->fadeToHue(data->led, data->hue);
        (*data->count)--;
    }

    free(data);
}

void Led::lightLeds(int16_t reviews, int16_t lessons)
{
    if (reviews < 0 && lessons < 0)
        return;

    static uint32_t prev = -1;

    //if ((reviews < 0 && lessons < 0) || prev == Utils::pair(reviews, lessons))
    //    return;
    // used to skip this, if it was called last time with the same arguments
    prev = Utils::pair(reviews, lessons);
    
    Serial.println("reviews & lessons: " + (String)reviews + " | " + (String)lessons);
    uint16_t ledsToLight = scaleToLeds(reviews + lessons);
    Serial.println("ledsToLight: " + (String)ledsToLight);
    int16_t reviewLeds = reviews ? ((float)reviews/(float)(reviews+lessons)) * ledsToLight : 0;
    int16_t lessonLeds = lessons ? ((float)lessons/(float)(reviews+lessons)) * ledsToLight : 0;
    Serial.println("reviewLeds & lessonLeds: " + (String)reviewLeds + " | " + (String)lessonLeds);
    
    // fill from the middle to the outside
    for( uint8_t y = 0; y < MATRIX_HEIGHT; y++)
    {
        for (uint8_t x=0; x <= MATRIX_WIDTH; x++)
        {
            if (lessonLeds > 0)
            {
                lightLed(&leds[ XY(xIterOrder[x], y) ], &lessonLeds, randomizeHue(HUE_LESSON), lessonLeds%2);
            }
            else if (reviewLeds > 0)
            {
                lightLed(&leds[ XY(xIterOrder[x], y) ], &reviewLeds, randomizeHue(HUE_REVIEW), lessonLeds%2);
            }
            else
            {
                fadeOff(&leds[ XY(xIterOrder[x], y) ]);
            }


            // FastLED.show();
            // delay(10);
        }
    }

    // leds[ XY(1, 0) ].setHSV(HUE_REVIEW, 255, 255);
    // leds[ XY(2, 2) ].setHSV(HUE_LESSON, 255, 255);

    // printLeds();
    FastLED.show();
}