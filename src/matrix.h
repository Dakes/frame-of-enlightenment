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

struct Coord
{
    Coord(uint8_t x, uint8_t y) : x(x), y(y) {}
    uint8_t x, y;
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
    void setCell(Coord coord, CellType type, uint32_t availableAt);
    Cell* getCell(Coord coord);
    Cell* getCell(uint8_t x, uint8_t y);
    const Cell* getCell(Coord coord) const;
    const Cell* getCell(uint8_t x, uint8_t y) const;
    static uint8_t width() { return MATRIX_WIDTH; }
    static uint8_t height() { return MATRIX_HEIGHT; }
    void simulationStep();

private:
    #define HUE_LESSON 220  // pink-ish 320°: 227
    #define HUE_REVIEW 142  // cyan-ish 200°: 142
    const uint8_t hueRandomness = 50;  // add a random value between +- this to the hue, for some variation
    // 0, 0 is bottom left
    Cell cells[MATRIX_WIDTH][MATRIX_HEIGHT];
    void generalCellLogic(Coord coord);
    void lessonCellLogic(Coord coord);
    void reviewCellLogic(Coord coord);

};

#endif

