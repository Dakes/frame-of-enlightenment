#include "matrix.h"

Matrix::Matrix(WaniKani *wk) { this->wk = wk; }

void Matrix::clear() {
  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
      cells[x][y] = Cell();
}

void Matrix::setCell(Coord coord, CellType type, uint8_t value) {
  const uint8_t x = coord.x;
  const uint8_t y = coord.y;
  if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT)
    return;
  cells[x][y].type = type;
  cells[x][y].value = value;
  switch (type) {
  case CELL_LESSON:
    cells[x][y].hue = HUE_LESSON;
    break;
  case CELL_REVIEW:
    cells[x][y].hue = HUE_REVIEW;
    break;
  case CELL_REVIEW_FUTURE:
    cells[x][y].hue = HUE_REVIEW_FUTURE;
    break;
  case CELL_EMPTY:
  default:
    cells[x][y].hue = 0;
    break;
  }

  cells[x][y].hue += random(-hueRandomness, hueRandomness + 1);
}

void Matrix::setCell(Coord coord, CellType type) {
  this->setCell(coord, type, V);
}

Cell *Matrix::getCell(uint8_t x, uint8_t y) {
  if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT || x < 0 || y < 0)
    return nullptr;
  return &cells[x][y];
}

Cell *Matrix::getCell(Coord coord) { return this->getCell(coord.x, coord.y); }

const Cell *Matrix::getCell(uint8_t x, uint8_t y) const {
  if (x >= MATRIX_WIDTH || y >= MATRIX_HEIGHT || x < 0 || y < 0)
    return nullptr;
  return &cells[x][y];
}

const Cell *Matrix::getCell(Coord coord) const {
  return this->getCell(coord.x, coord.y);
}

/**
 * Performs a simulation step for every cell in the Matrix
 */
void Matrix::simulationStep() {
  static uint8_t calls = 0;
  calls++;
  if (calls < MATRIX_STEP_FRAMES)
    return;

  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x) {
    for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y) {
      Cell cell = cells[x][y];
      Coord coord = Coord(x, y);
      this->generalCellLogic(coord);
      if (cell.type == CELL_LESSON) {
        this->lessonCellLogic(coord);
      } else if (cell.type == CELL_REVIEW) {
        this->reviewCellLogic(coord);
      }
    }
  }
  calls = 0;
}

void Matrix::generalCellLogic(Coord coord) {
  Cell *cur = getCell(coord);
  if (!cur || cur->type == CELL_EMPTY || cur->type == CELL_REVIEW_FUTURE)
    return;

  const uint8_t x = coord.x;
  const uint8_t y = coord.y;

  // Try to move straight down
  Cell *below = (y > 0) ? getCell(x, (uint8_t)(y - 1))
                        : nullptr; // y==0 -> boundary (nullptr)
  if (below && below->type == CELL_EMPTY) {
    *below = *cur;
    *cur = Cell(); // leave an empty cell behind
    return;
  }

  // If blocked below, try diagonals
  Cell *downLeft =
      (y > 0 && x > 0) ? getCell((uint8_t)(x - 1), (uint8_t)(y - 1)) : nullptr;
  Cell *downRight = (y > 0 && (uint8_t)(x + 1) < MATRIX_WIDTH)
                        ? getCell((uint8_t)(x + 1), (uint8_t)(y - 1))
                        : nullptr;

  const bool leftFree = (downLeft && downLeft->type == CELL_EMPTY);
  const bool rightFree = (downRight && downRight->type == CELL_EMPTY);

  if (leftFree && rightFree) {
    // Simple randomness: millis() LSB
    if ((millis() & 1) == 0)
      *downLeft = *cur;
    else
      *downRight = *cur;

    *cur = Cell();
  } else if (leftFree) {
    *downLeft = *cur;
    *cur = Cell();
  } else if (rightFree) {
    *downRight = *cur;
    *cur = Cell();
  }
  // else: stays in place (rests on floor or another cell)
}

void Matrix::lessonCellLogic(Coord coord) {}

void Matrix::reviewCellLogic(Coord coord) {}

/**
 * Code for top row for future Reviews
 */
void Matrix::updateReviewFutureRow() {
  // only do computation if source data changed
  ulong static lastUpdate = 0;
  if (wk->lastRequestTime <= lastUpdate)
    return;
  lastUpdate = millis();

  // get max reviews for all future hours to normalize the brightness
  uint16_t maxReviews = 0;
  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x) {
    uint16_t reviewsHour = this->getScaledFutureReview(x);
    if (reviewsHour > maxReviews)
      maxReviews = reviewsHour;
  }
  if (maxReviews == 0) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
      this->setCell(Coord(x, MATRIX_HEIGHT - 1), CELL_REVIEW_FUTURE, 0);
    return;
  }

  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x) {
    Coord coord = Coord(x, MATRIX_HEIGHT - 1);
    uint16_t reviewsHour = this->getScaledFutureReview(x);
    uint8_t normalizedBrightness = (reviewsHour * V) / maxReviews;
    this->setCell(coord, CELL_REVIEW_FUTURE, normalizedBrightness);
  }
}

/**
 * Get number of reviews per pixel. can be multiple hours per pixel after
 * ONE_TO_ONE_HOURS
 * @param x coord
 * @return number of reviews
 */
uint16_t Matrix::getScaledFutureReview(uint8_t x) {
  if (x < ONE_TO_ONE_HOURS) {
    int16_t reviewsHour = wk->getReviews(x + 1);
    return reviewsHour > 0 ? reviewsHour : 0;
  }

  const uint8_t pixelsLeft = MATRIX_WIDTH - ONE_TO_ONE_HOURS;
  if (pixelsLeft == 0)
    return 0;
  const uint8_t hoursPerPixel =
      ((24 - ONE_TO_ONE_HOURS) + pixelsLeft / 2) / pixelsLeft;

  uint16_t reviewsPixel = 0;
  uint8_t startHour = (x - ONE_TO_ONE_HOURS) * hoursPerPixel + ONE_TO_ONE_HOURS;
  for (uint8_t hour = startHour; hour < startHour + hoursPerPixel; ++hour) {
    int16_t hourCount = wk->getReviews(hour + 1);
    if (hourCount > 0)
      reviewsPixel += hourCount;
  }
  return reviewsPixel;
}

ReviewLessonCounts Matrix::getReviewLessonCounts() const {
  ReviewLessonCounts counts;

  for (const auto &cell : cells) {
    for (auto c : cell) {
      if (c.type == CELL_LESSON)
        ++counts.lessons;
      else if (c.type == CELL_REVIEW)
        ++counts.reviews;
    }
  }

  return counts;
}

Coord Matrix::getRandomCellCoord(CellType type) const {
  uint16_t total = 0;
  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
      if (cells[x][y].type == type)
        ++total;

  if (total == 0)
    return Coord(0, 0);

  uint16_t target = random(total);
  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x) {
    for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y) {
      if (cells[x][y].type == type) {
        if (target == 0)
          return Coord(x, y);
        --target;
      }
    }
  }
  return Coord(0, 0);
}

bool Matrix::removeRandomCell(CellType type) {
  Coord coord = getRandomCellCoord(type);
  Cell *cell = getCell(coord);
  if (!cell || cell->type != type)
    return false;
  *cell = Cell();
  return true;
}

void Matrix::spawnCellAtTop(CellType type, uint8_t value) {
  // Choose a starting column around the center
  uint8_t startX = MATRIX_WIDTH / 2;
  if ((MATRIX_WIDTH & 1) == 0)
    startX = (random(2) == 0) ? startX : (uint8_t)(startX - 1);

  for (int8_t y = MATRIX_HEIGHT - 1; y >= 0; --y) {
    bool rightFirst = random(2);
    for (uint8_t offset = 0; offset < MATRIX_WIDTH; ++offset) {
      int16_t x = startX + (rightFirst ? 1 : -1) * offset;
      if (x >= 0 && x < MATRIX_WIDTH &&
          (cells[x][y].type == CELL_EMPTY ||
           cells[x][y].type == CELL_REVIEW_FUTURE)) {
        setCell(Coord(x, y), type, value);
        return;
      }
      if (offset != 0) {
        x = startX - (rightFirst ? 1 : -1) * offset;
        if (x >= 0 && x < MATRIX_WIDTH &&
            (cells[x][y].type == CELL_EMPTY ||
             cells[x][y].type == CELL_REVIEW_FUTURE)) {
          setCell(Coord(x, y), type, value);
          return;
        }
      }
    }
    // row is full, try the next row down
  }
}

void Matrix::setBrightnessOne(CellType type, uint8_t value) {
  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x) {
    for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y) {
      if (cells[x][y].type == type) {
        cells[x][y].value = value;
        return;
      }
    }
  }
}

void Matrix::setBrightnessAll(CellType type, uint8_t value) {
  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
    for (uint8_t y = 0; y < MATRIX_HEIGHT; ++y)
      if (cells[x][y].type == type)
        cells[x][y].value = value;
}

void Matrix::checkReviewLessonCounts() {
  static ulong lastSpawn = 0;

  int16_t wkLessons = wk->getLessons();
  int16_t wkReviews = wk->getReviews();
  if (wkLessons < 0)
    wkLessons = 0;
  if (wkReviews < 0)
    wkReviews = 0;
  if (wkLessons > FRAME_FULL)
    wkLessons = FRAME_FULL;
  if (wkReviews > FRAME_FULL)
    wkReviews = FRAME_FULL;

  auto process = [&](CellType type, int16_t count, uint16_t cellCount) {
    if (count == 0) {
      if (cellCount > 0)
        removeRandomCell(type);
      return;
    }

    if (count < ITEMS_PER_PIXEL) {
      if (cellCount == 0) {
        if (millis() - lastSpawn >= SPAWN_DELAY) {
          uint8_t brightness = (count * V) / ITEMS_PER_PIXEL;
          spawnCellAtTop(type, brightness);
          lastSpawn = millis();
        }
      } else if (cellCount == 1) {
        uint8_t brightness = (count * V) / ITEMS_PER_PIXEL;
        setBrightnessOne(type, brightness);
      } else // cellCount > 1
      {
        removeRandomCell(type);
      }
      return;
    }

    // count >= ITEMS_PER_PIXEL
    setBrightnessAll(type, V);
    uint16_t requiredCells = count / ITEMS_PER_PIXEL;
    if (cellCount > requiredCells) {
      removeRandomCell(type);
      return;
    }
    if (cellCount < requiredCells) {
      if (millis() - lastSpawn >= SPAWN_DELAY) {
        spawnCellAtTop(type, V);
        lastSpawn = millis();
      }
      return;
    }
    // else exact match; leftover items ignored
  };

  ReviewLessonCounts counts = getReviewLessonCounts();
  process(CELL_REVIEW, wkReviews, counts.reviews);
  counts = getReviewLessonCounts();
  process(CELL_LESSON, wkLessons, counts.lessons);
}
