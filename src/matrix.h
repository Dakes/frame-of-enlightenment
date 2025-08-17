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
    uint32_t availableAt = 0; // hour when available
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

private:
    Cell cells[MATRIX_WIDTH][MATRIX_HEIGHT];
};

#endif

