#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#define SQUARE_SIDE ((size_t) 3)
#define BOARD_SIZE ((size_t) SQUARE_SIDE * SQUARE_SIDE)

typedef uint16_t Cell;
static_assert(sizeof(Cell) * 8 >= BOARD_SIZE, "Cell width not big enough!");

static const Cell UNKNOWN = (1 << BOARD_SIZE) - 1;

bool stop(Cell board[/*BOARD_SIZE*/][BOARD_SIZE]);
bool isValid(Cell board[/*BOARD_SIZE*/][BOARD_SIZE]);

void column(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t c, Cell* set[/*BOARD_SIZE*/]);
void row(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t r, Cell* set[/*BOARD_SIZE*/]);
void block(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t b, Cell* set[/*BOARD_SIZE*/]);

void solveByCells(Cell* cells[/*BOARD_SIZE*/]);
void solveByPossibilities(Cell* cells[/*BOARD_SIZE*/]);

int bitNum(Cell c) { for (int i = 0; ; ++i) if ((1 << (i - 1)) >= c) return i; }
void printBoard(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], bool pretty)
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    for (size_t j = 0; j < BOARD_SIZE; ++j) {
      printf("%i, ", pretty ? bitNum(board[i][j]) : board[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

static bool PRINT_DEBUG = false;

int main(int argc, char* argv[])
{
  const char* board_str;
  if (argc == 2) {
    board_str = argv[1];
  } else if (argc == 3 && argv[1][0] == '-') {
    board_str = argv[2];
    for (const char* o = &argv[1][1]; *o != '\0'; ++o) {
      switch (*o) {
        case 'd': PRINT_DEBUG = true; break;
        default: printf("Unknown opt '%c'.\n", *o); return 1;
      }
    }
  } else {
    printf("Bad args!\n");
    return 1;
  }

  Cell board[BOARD_SIZE][BOARD_SIZE];
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    for (size_t j = 0; j < BOARD_SIZE; ++j) {
      const char c = board_str[i * BOARD_SIZE + j];
      if (c < '0') {
	printf("Invalid board char '%c'.\n", c);
	return 1;
      }
      const uint8_t v = c - '0';
      if (v > BOARD_SIZE) {
	printf("Invalid board value '%i' from char '%c'.\n", v, c);
	return 1;
      }
      board[i][j] = v == 0 ? UNKNOWN : 1 << (v - 1);
    }
  }

  typedef void (*SetGetter)(Cell /*board*/[][BOARD_SIZE], size_t /*idx*/, Cell* /*res*/[]);
  SetGetter set_getters[] = {column, row, block, NULL};

  int num_iters = 0;
  while (!stop(board)) {
    Cell* cells[BOARD_SIZE];
    for (SetGetter* getSet = &set_getters[0]; *getSet != NULL; ++getSet) {
      for (size_t set_i = 0; set_i < BOARD_SIZE; ++set_i) {
	(*getSet)(board, set_i, cells);

	if (PRINT_DEBUG) {
	  printBoard(board, false);
	  printf("Cells: %s %zd\n",
		 *getSet == column ? "Col" : *getSet == row ? "Row" : "Block", set_i + 1);
	}
	solveByCells(cells);

	if (PRINT_DEBUG) {
	  printBoard(board, false);
	  printf("Possibilites: %s %zd\n",
		 *getSet == column ? "Col" : *getSet == row ? "Row" : "Block", set_i + 1);
	}
	solveByPossibilities(cells);
      }
    }
    ++num_iters;
  }

  const bool valid = isValid(board);
  printf("%s result found! Took %i iterations.\n", valid ? "Valid" : "Invalid", num_iters);
  printBoard(board, valid);
}

void column(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t c, Cell* set[/*BOARD_SIZE*/])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) set[i] = &board[i][c];
}

void row(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t r, Cell* set[/*BOARD_SIZE*/])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) set[i] = &board[r][i];
}

void block(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t b, Cell* set[/*BOARD_SIZE*/])
{
  const size_t s = SQUARE_SIDE;
  for (size_t i = 0; i < BOARD_SIZE; ++i) set[i] = &board[b / s * s + i / s][b % s * s + i % s];
}

int countBits(uint16_t val);

bool stop(Cell board[/*BOARD_SIZE*/][BOARD_SIZE])
{
  static Cell prev_board[BOARD_SIZE][BOARD_SIZE] = { { 0 } };
  bool res = true;
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    for (size_t j = 0; j < BOARD_SIZE; ++j) {
      if (prev_board[i][j] != board[i][j] && countBits(board[i][j]) != 1) {
	res = false;
	prev_board[i][j] = board[i][j];
      }
    }
  }
  return res;
}

bool isValid(Cell board[/*BOARD_SIZE*/][BOARD_SIZE])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    for (size_t j = 0; j < BOARD_SIZE; ++j) {
      if (countBits(board[i][j]) != 1) {
	return false;
      }
    }
  }
  return true;
}

void solveByCells(Cell* cells[/*BOARD_SIZE*/])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    bool subsets[BOARD_SIZE] = { false };
    int num_subsets = 0;

    const uint16_t vals = *cells[i];
    for (size_t j = i; j < BOARD_SIZE; ++j) {
      if ((*cells[j] | vals) == vals) {
	subsets[j] = true;
	++num_subsets;
      }
    }

    if (countBits(vals) == num_subsets) {
      bool changed = false;
      for (size_t j = 0; j < BOARD_SIZE; ++j) {
	const Cell old = *cells[j];
	*cells[j] &= subsets[j] ? vals : ~vals & UNKNOWN;
	if (*cells[j] != old) changed = true;
      }
      if (PRINT_DEBUG && changed) printf("Set of %i cells fulfilling %i.\n", num_subsets, vals);
    }
  }
}

uint16_t transposeBits(Cell* const cells[], size_t bit_i);

void solveByPossibilities(Cell* cells[/*BOARD_SIZE*/])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    uint16_t mask = 0;
    int num_subsets = 0;

    const uint16_t vals = transposeBits(cells, i);
    for (size_t j = i; j < BOARD_SIZE; ++j) {
      if ((transposeBits(cells, j) | vals) == vals) {
	mask |= 1 << j;
	++num_subsets;
      }
    }

    if (countBits(vals) == num_subsets) {
      bool changed = false;
      for (size_t j = 0; j < BOARD_SIZE; ++j) {
	const Cell old = *cells[j];
        *cells[j] &= ((1 << j) & vals) ? mask : ~mask & UNKNOWN;
	if (*cells[j] != old) changed = true;
      }
      if (PRINT_DEBUG && changed) printf("Set of %i possibilities fulfilled by %i.\n", num_subsets, mask);
    }
  }
}

int countBits(uint16_t val)
{
  int res = 0;
  for (size_t i = 0; i < BOARD_SIZE; ++i) if (val & (1 << i)) ++res;
  return res;
}

uint16_t transposeBits(Cell* const cells[], size_t bit_i)
{
  uint16_t res = 0;
  for (size_t i = 0; i < BOARD_SIZE; ++i) if (*cells[i] & (1 << bit_i)) res |= 1 << i;
  return res;
}
