#include "matrix.h"

Matrix::Matrix()
{
    clear();
}

void Matrix::clear()
{
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
        for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
            cells[x][y] = Cell();
}

void Matrix::setCell(Coord coord, CellType type, uint8_t value)
{
    const uint8_t x = coord.x;
    const uint8_t y = coord.y;
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT)
        return;
    cells[x][y].type = type;
    cells[x][y].value = value;
    cells[x][y].hue = type == CELL_LESSON ? HUE_LESSON : HUE_REVIEW;
    cells[x][y].hue += random(-hueRandomness, hueRandomness+1);
}


void Matrix::setCell(Coord coord, CellType type)
{
    this->setCell(coord, type, V);
}

Cell* Matrix::getCell(uint8_t x, uint8_t y)
{
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT || x < 0 || y < 0)
        return nullptr;
    return &cells[x][y];
}

Cell* Matrix::getCell(Coord coord)
{
    return this->getCell(coord.x, coord.y);
}

const Cell* Matrix::getCell(uint8_t x, uint8_t y) const
{
    if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT || x < 0 || y < 0)
        return nullptr;
    return &cells[x][y];
}

const Cell* Matrix::getCell(Coord coord) const
{
    return this->getCell(coord.x, coord.y);
}


/**
 * Performs a simulation step for every cell in the Matrix
 */
void Matrix::simulationStep()
{
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    {
        for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
        {
            Cell cell = cells[x][y];
            Coord coord = Coord(x, y);
            this->generalCellLogic(coord);
            if (cell.type == CELL_LESSON)
            {
                this->lessonCellLogic(coord);
            }
            else if (cell.type == CELL_REVIEW)
            {
                this->reviewCellLogic(coord);
            }
        }
    }
}

void Matrix::generalCellLogic(Coord coord)
{
    Cell* cur = getCell(coord);
    if (!cur || cur->type == CELL_EMPTY || cur->type == CELL_REVIEW_FUTURE)
        return;

    const uint8_t x = coord.x;
    const uint8_t y = coord.y;

    // Try to move straight down
    Cell* below = (y > 0) ? getCell(x, (uint8_t)(y - 1)) : nullptr;  // y==0 -> boundary (nullptr)
    if (below && below->type == CELL_EMPTY)
    {
        *below = *cur;
        *cur = Cell();  // leave an empty cell behind
        return;
    }

    // If blocked below, try diagonals
    Cell* downLeft  = (y > 0 && x > 0) ? getCell((uint8_t)(x - 1), (uint8_t)(y - 1)) : nullptr;
    Cell* downRight = (y > 0 && (uint8_t)(x + 1) < MATRIX_WIDTH) ? getCell((uint8_t)(x + 1), (uint8_t)(y - 1)) : nullptr;

    const bool leftFree  = (downLeft  && downLeft->type  == CELL_EMPTY);
    const bool rightFree = (downRight && downRight->type == CELL_EMPTY);

    if (leftFree && rightFree)
    {
        // Simple randomness: millis() LSB
        if ((millis() & 1) == 0)
            *downLeft = *cur;
        else
            *downRight = *cur;

        *cur = Cell();
    }
    else if (leftFree)
    {
        *downLeft = *cur;
        *cur = Cell();
    }
    else if (rightFree)
    {
        *downRight = *cur;
        *cur = Cell();
    }
    // else: stays in place (rests on floor or another cell)
}


void Matrix::lessonCellLogic(Coord coord)
{

}

void Matrix::reviewCellLogic(Coord coord)
{

}

// Code for top row for future Reviews

void Matrix::updateReviewFutureRow(WaniKani wk)
{
    ulong static lastUpdate = 0;
    if (wk.lastRequestTime <= lastUpdate)
        return;
    lastUpdate = millis();

    uint16_t maxReviews = 0;
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    {
        uint16_t reviewsHour = wk.getReviews(x+1);
        if (reviewsHour > maxReviews)
            maxReviews = reviewsHour;
    }

    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    {
        Serial.println(" x: " + String(x));
        Coord coord = Coord(x, MATRIX_HEIGHT-1);
        uint16_t reviewsHour = wk.getReviews(x+1);
        Serial.println(" reviewsHour: " + String(reviewsHour));
        uint8_t normalizedBrightness = (reviewsHour * V) / maxReviews;
        this->setCell(coord, CELL_REVIEW_FUTURE, normalizedBrightness);
    }

}
