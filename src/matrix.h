#ifndef MATRIX_H
#define MATRIX_H

#include <Arduino.h>
#include "led.h"
#include "wanikani.h"

#define ONE_TO_ONE_HOURS 3  // number of pixels to light for 1 hour, rest will be squished to 24h

enum CellType
{
    CELL_EMPTY,
    CELL_LESSON,
    CELL_REVIEW,
    CELL_REVIEW_FUTURE
};

struct Coord
{
    Coord(uint8_t x, uint8_t y) : x(x), y(y) {}
    uint8_t x, y;
};

struct Cell
{
    CellType type = CELL_EMPTY;
    u8_t hue = 0;  // hue of the color, because the hue varies for a better visual effect
    uint8_t value = V;  // brightness value. mostly used for the future row
};

class Matrix
{
public:
    Matrix(WaniKani* wk);
    void clear();
    void setCell(Coord coord, CellType type, uint8_t value);
    void setCell(Coord coord, CellType type);
    Cell* getCell(Coord coord);
    Cell* getCell(uint8_t x, uint8_t y);
    const Cell* getCell(Coord coord) const;
    const Cell* getCell(uint8_t x, uint8_t y) const;
    static uint8_t width() { return MATRIX_WIDTH; }
    static uint8_t height() { return MATRIX_HEIGHT; }
    void simulationStep();

    void updateReviewFutureRow();

private:
    WaniKani* wk;
    const uint8_t hueRandomness = 25;  // add a random value between +- this to the hue, for some variation
    // 0, 0 is bottom left
    Cell cells[MATRIX_WIDTH][MATRIX_HEIGHT];
    void generalCellLogic(Coord coord);
    void lessonCellLogic(Coord coord);
    void reviewCellLogic(Coord coord);
    uint16_t getScaledFutureReview(uint8_t x);

};

#endif

