#include "matrix.h"

Matrix::Matrix(WaniKani *wk) {
  this->wk = wk;
  cells = nullptr;
}

void Matrix::init() {
  cells = new Cell[MATRIX_INTERNAL_WIDTH * MATRIX_INTERNAL_HEIGHT];
  clear();
}

void Matrix::clear() {
  for (uint8_t x = 0; x < MATRIX_INTERNAL_WIDTH; ++x)
    for (uint8_t y = 0; y < MATRIX_INTERNAL_HEIGHT; ++y)
      cellAt(x, y) = Cell();
}

void Matrix::setCell(Coord coord, CellType type, uint8_t value) {
  const uint8_t x = coord.x;
  const uint8_t y = coord.y;
  if (x >= MATRIX_INTERNAL_WIDTH || y >= MATRIX_INTERNAL_HEIGHT)
    return;
  cellAt(x, y).type = type;
  cellAt(x, y).value = value;
  switch (type) {
  case CELL_LESSON:
    cellAt(x, y).hue = HUE_LESSON;
    break;
  case CELL_REVIEW:
    cellAt(x, y).hue = HUE_REVIEW;
    break;
  case CELL_REVIEW_FUTURE:
    cellAt(x, y).hue = HUE_REVIEW_FUTURE;
    break;
  case CELL_EMPTY:
  default:
    cellAt(x, y).hue = 0;
    break;
  }

  cellAt(x, y).hue += random(-hueRandomness, hueRandomness + 1);
}

void Matrix::setCell(Coord coord, CellType type) {
  this->setCell(coord, type, V);
}

Cell *Matrix::getCell(uint8_t x, uint8_t y) {
  if (x >= MATRIX_INTERNAL_WIDTH || y >= MATRIX_INTERNAL_HEIGHT || x < 0 || y < 0)
    return nullptr;
  return &cellAt(x, y);
}

Cell *Matrix::getCell(Coord coord) { return this->getCell(coord.x, coord.y); }

const Cell *Matrix::getCell(uint8_t x, uint8_t y) const {
  if (x >= MATRIX_INTERNAL_WIDTH || y >= MATRIX_INTERNAL_HEIGHT || x < 0 || y < 0)
    return nullptr;
  return &cellAt(x, y);
}

const Cell *Matrix::getCell(Coord coord) const {
  return this->getCell(coord.x, coord.y);
}

CHSV Matrix::getLedColor(uint8_t x, uint8_t y) const {
    // Accumulate hue on the unit circle, weighted by value (V)
    int32_t vecX = 0;
    int32_t vecY = 0;
    uint32_t sumV = 0;

    for (uint8_t sx = 0; sx < MATRIX_RESOLUTION; ++sx) {
        for (uint8_t sy = 0; sy < MATRIX_RESOLUTION; ++sy) {
            const uint8_t ix = x * MATRIX_RESOLUTION + sx;
            const uint8_t iy = y * MATRIX_RESOLUTION + sy;
            const Cell &cell = cellAt(ix, iy);

            if (cell.type != CELL_EMPTY) {
                const uint8_t h = cell.hue;
                const uint8_t v = cell.value;

                const int16_t cx = (int16_t)cos8(h) - 128;
                const int16_t cy = (int16_t)sin8(h) - 128;

                vecX += (int32_t)cx * v;
                vecY += (int32_t)cy * v;
                sumV += v;
            }
        }
    }

    const uint16_t total = MATRIX_RESOLUTION * MATRIX_RESOLUTION;

    // Average value across the full supersample grid (empties contribute 0)
    const uint8_t avgV = (uint8_t)(sumV / total);

    // Compute average hue from vector; default to 0 if vector is zero
    uint8_t avgH = 0;
    if (vecX != 0 || vecY != 0) {
        float angle = (float)atan2((float)vecY, (float)vecX); // -pi..pi
        if (angle < 0.0f) {
            angle += 6.283185307f; // +2*pi
        }
        // Map 0..2*pi to 0..255
        avgH = (uint8_t)(angle * 40.584510488f); // 255 / (2*pi)
    }

    // Keep saturation constant
    return CHSV(avgH, S, avgV);
}


/**
 * Performs a simulation step for every cell in the Matrix
 */
void Matrix::simulationStep() {
  static uint8_t calls = 0;
  calls++;
  if (calls < MATRIX_STEP_FRAMES)
    return;

  for (uint8_t x = 0; x < MATRIX_INTERNAL_WIDTH; ++x) {
    for (uint8_t y = 0; y < MATRIX_INTERNAL_HEIGHT; ++y) {
      Cell cell = cellAt(x, y);
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
  Cell *downRight = (y > 0 && (uint8_t)(x + 1) < MATRIX_INTERNAL_WIDTH)
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

void Matrix::lessonCellLogic(Coord coord) {
  Cell *cur = getCell(coord);
  if (!cur || cur->type != CELL_LESSON)
    return;

  const uint8_t x = coord.x;
  const uint8_t y = coord.y;

  Cell *downLeft2 =
      (y > 0 && x > 1) ? getCell((uint8_t)(x - 2), (uint8_t)(y - 1)) : nullptr;
  Cell *downRight2 =
      (y > 0 && (uint8_t)(x + 2) < MATRIX_INTERNAL_WIDTH)
          ? getCell((uint8_t)(x + 2), (uint8_t)(y - 1))
          : nullptr;

  const bool leftFree = (downLeft2 && downLeft2->type == CELL_EMPTY);
  const bool rightFree = (downRight2 && downRight2->type == CELL_EMPTY);

  if (leftFree && rightFree) {
    if ((millis() & 1) == 0)
      *downLeft2 = *cur;
    else
      *downRight2 = *cur;

    *cur = Cell();
  } else if (leftFree) {
    *downLeft2 = *cur;
    *cur = Cell();
  } else if (rightFree) {
    *downRight2 = *cur;
    *cur = Cell();
  }
}

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
  uint8_t startY = (MATRIX_HEIGHT - 1) * MATRIX_RESOLUTION;
  if (maxReviews == 0) {
    for (uint8_t x = 0; x < MATRIX_WIDTH; ++x)
      for (uint8_t sx = 0; sx < MATRIX_RESOLUTION; ++sx)
        for (uint8_t sy = 0; sy < MATRIX_RESOLUTION; ++sy) {
          Coord c(x * MATRIX_RESOLUTION + sx, startY + sy);
          Cell *cell = getCell(c);
          if (cell && cell->type == CELL_REVIEW_FUTURE)
            setCell(c, CELL_REVIEW_FUTURE, 0);
        }
    return;
  }

  for (uint8_t x = 0; x < MATRIX_WIDTH; ++x) {
    uint16_t reviewsHour = this->getScaledFutureReview(x);
    uint8_t normalizedBrightness = (reviewsHour * V) / maxReviews;
    for (uint8_t sx = 0; sx < MATRIX_RESOLUTION; ++sx)
      for (uint8_t sy = 0; sy < MATRIX_RESOLUTION; ++sy) {
        Coord c(x * MATRIX_RESOLUTION + sx, startY + sy);
        Cell *cell = getCell(c);
        if (cell && (cell->type == CELL_EMPTY || cell->type == CELL_REVIEW_FUTURE))
          setCell(c, CELL_REVIEW_FUTURE, normalizedBrightness);
      }
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
  for (uint8_t x = 0; x < MATRIX_INTERNAL_WIDTH; ++x) {
    for (uint8_t y = 0; y < MATRIX_INTERNAL_HEIGHT; ++y) {
      const Cell &c = cellAt(x, y);
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
  for (uint8_t x = 0; x < MATRIX_INTERNAL_WIDTH; ++x)
    for (uint8_t y = 0; y < MATRIX_INTERNAL_HEIGHT; ++y)
      if (cellAt(x, y).type == type)
        ++total;

  if (total == 0)
    return Coord(0, 0);

  uint16_t target = random(total);
  for (uint8_t x = 0; x < MATRIX_INTERNAL_WIDTH; ++x) {
    for (uint8_t y = 0; y < MATRIX_INTERNAL_HEIGHT; ++y) {
      if (cellAt(x, y).type == type) {
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
  const uint8_t STAY_PERCENTAGE = 99;

  static int8_t targetX = -1;
  enum SpawnState { STAY, SEARCH };
  static SpawnState state = SEARCH;

  uint8_t startX;
  if (state == STAY && targetX >= 0) {
    startX = targetX;
  } else {
    startX = MATRIX_INTERNAL_WIDTH / 2;
    if ((MATRIX_INTERNAL_WIDTH & 1) == 0)
      startX = (random(2) == 0) ? startX : (uint8_t)(startX - 1);
  }

  for (int16_t y = MATRIX_INTERNAL_HEIGHT - 1; y >= 0; --y) {
    bool rightFirst = random(2);
    for (uint8_t offset = 0; offset < MATRIX_INTERNAL_WIDTH; ++offset) {
      int16_t x = startX + (rightFirst ? 1 : -1) * offset;
      if (x >= 0 && x < MATRIX_INTERNAL_WIDTH && cellAt(x, y).type == CELL_EMPTY) {
        setCell(Coord(x, y), type, value);
        targetX = x;
        state = (random(100) < STAY_PERCENTAGE) ? STAY : SEARCH;
        return;
      }
      if (offset != 0) {
        x = startX - (rightFirst ? 1 : -1) * offset;
        if (x >= 0 && x < MATRIX_INTERNAL_WIDTH && cellAt(x, y).type == CELL_EMPTY) {
          setCell(Coord(x, y), type, value);
          targetX = x;
          state = (random(100) < STAY_PERCENTAGE) ? STAY : SEARCH;
          return;
        }
      }
    }
    // row is full, try the next row down
  }
}


void Matrix::checkReviewLessonCounts() {
  static ulong lastSpawn = 0;
  enum SpawnOrder { REVIEWS_FIRST, LESSONS_FIRST };
  static SpawnOrder order = REVIEWS_FIRST;
  const uint8_t REVIEW_TO_LESSON_PER_MYRIAD = 8;
  const uint8_t LESSON_TO_REVIEW_PER_MYRIAD = 10;

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
    uint32_t cellsPerPixel = MATRIX_RESOLUTION * MATRIX_RESOLUTION;
    uint32_t requiredCells = ((uint32_t)count * cellsPerPixel) / ITEMS_PER_PIXEL;
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
  if (order == REVIEWS_FIRST) {
    process(CELL_REVIEW, wkReviews, counts.reviews);
    counts = getReviewLessonCounts();
    process(CELL_LESSON, wkLessons, counts.lessons);
    if (random(10000) < REVIEW_TO_LESSON_PER_MYRIAD)
      order = LESSONS_FIRST;
  } else {
    process(CELL_LESSON, wkLessons, counts.lessons);
    counts = getReviewLessonCounts();
    process(CELL_REVIEW, wkReviews, counts.reviews);
    if (random(10000) < LESSON_TO_REVIEW_PER_MYRIAD)
      order = REVIEWS_FIRST;
  }
}
