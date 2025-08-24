#include "led.h"
#include "matrix.h"

Led::Led()
{
}

void Led::begin()
{
    this->setup();
}

void Led::setup()
{
    Serial.println("\n--- LED Setup ---");
    Serial.print("Initializing FastLED with ");
    Serial.print(NUM_LEDS);
    Serial.print(" LEDs on pin ");
    Serial.println(DATA_PIN);

    FastLED.addLeds<LED_TYPE, DATA_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(TypicalLEDStrip);
    FastLED.clear();
    FastLED.show(); // Explicitly show the cleared state

    Serial.println("LED setup complete");

    // Set full brightness to start
    FastLED.setBrightness(255);
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
            else // Even rows run forwards
                i = (y * MATRIX_WIDTH) + x;
        }
        else
        {
            // vertical positioning
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
    for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
    {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; y++)
        {
            // Here's the x, y to 'led index' in action:
            leds[XY(x, y)].setHSV(random8(), 255, 255);

            FastLED.show();
            delay(100);
        }
    }
    delay(500);
    FastLED.clear();
}

void Led::printLeds()
{
    Serial.println("Led matrix: ");
    for (uint8_t y = MATRIX_HEIGHT; y > 0; y--)
    {
        for (uint8_t x = 0; x < MATRIX_WIDTH; x++)
        {
            int led = leds[XY(x, y)][0] + leds[XY(x, y)][1] + leds[XY(x, y)][2] + leds[XY(x, y)][3];
            if (led < 10) Serial.print(" ");
            if (led < 100) Serial.print(" ");
            Serial.print(led);
            Serial.print("|");
        }
        Serial.println();
    }
    Serial.println();
}

/**
 * Fade to targetHue smoothly.
 * pixel is a pointer to the individual pixel from Fastled led matrix.
 * If pixel is off, fades it to on at that hue from the beginning.
*/
void Led::displayMatrix(const Matrix& matrix)
{
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
        {
            CHSV color = matrix.getLedColor(x, y);
            CRGB color_rgb{};
            hsv2rgb_spectrum(color, color_rgb);
            leds[XY(x, y)] = color_rgb;

            // Map luma 0..255 to MIN_V..255
            const uint8_t v = color.v;
            if (v > 0)
            {
                const uint8_t mappedV = MIN_V + scale8(v, 255 - MIN_V);

                // Convert to HSV, replace V with mappedV, convert back to RGB
                color.v = mappedV;
                hsv2rgb_spectrum(color, color_rgb);
                leds[XY(x, y)] = color_rgb;
            }
        }
    }

    FastLED.show();
}
