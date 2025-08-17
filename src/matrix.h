#ifndef MATRIX_H
#define MATRIX_H

#include <Arduino.h>
#include "led.h"

enum CellType
{
    CELL_EMPTY,
    CELL_LESSON,
    CELL_REVIEW
};

struct Cell
{
    CellType type = CELL_EMPTY;
    uint32_t availableAt = 0;  // hour when available
    u8_t hue = 0;  // hue of the color, because the hue varies for a better visual effect
};

class Matrix
{
public:
    Matrix();
    void clear();
    void setCell(uint8_t x, uint8_t y, CellType type, uint32_t availableAt);
    Cell getCell(uint8_t x, uint8_t y) const;
    uint8_t width() const { return MATRIX_WIDTH; }
    uint8_t height() const { return MATRIX_HEIGHT; }
    void simulationStep();

private:
    #define HUE_LESSON 220  // pink-ish 320°: 227
    #define HUE_REVIEW 142  // cyan-ish 200°: 142
    const uint8_t hueRandomness = 50;  // add a random value between +- this to the hue, for some variation
    Cell cells[MATRIX_WIDTH][MATRIX_HEIGHT];
};

#endif

