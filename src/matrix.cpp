#include "matrix.h"

Matrix::Matrix()
{
    clear();
}

void Matrix::clear()
{
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
        {
            cells[x][y] = Cell();
        }
    }
}

void Matrix::setCell(uint8_t x, uint8_t y, CellType type, uint32_t availableAt)
{
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT)
        return;
    cells[x][y].type = type;
    cells[x][y].availableAt = availableAt;
}

Cell Matrix::getCell(uint8_t x, uint8_t y) const
{
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT)
        return Cell();
    return cells[x][y];
}
