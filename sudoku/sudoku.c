#include <assert.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>

#ifdef CUSTOM_SQUARE_SIDE
# define SQUARE_SIDE ((size_t) (CUSTOM_SQUARE_SIDE))
#else
# define SQUARE_SIDE ((size_t) 3)
#endif

#define BOARD_SIZE ((size_t) (SQUARE_SIDE * SQUARE_SIDE))

typedef uint16_t Cell;
static_assert(sizeof(Cell) * 8 > BOARD_SIZE, "Cell width not big enough!");

static const Cell UNKNOWN = (1 << BOARD_SIZE) - 1;

void copyBoard(const Cell src[/*BOARD_SIZE*/][BOARD_SIZE], Cell dest[/*BOARD_SIZE*/][BOARD_SIZE]);
bool changed(const Cell src[/*BOARD_SIZE*/][BOARD_SIZE], Cell dest[/*BOARD_SIZE*/][BOARD_SIZE]);
bool isComplete(const Cell board[/*BOARD_SIZE*/][BOARD_SIZE]);

void column(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t c, Cell* set[/*BOARD_SIZE*/]);
void row(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t r, Cell* set[/*BOARD_SIZE*/]);
void block(Cell board[/*BOARD_SIZE*/][BOARD_SIZE], size_t b, Cell* set[/*BOARD_SIZE*/]);

bool solveNaked(Cell board[/*BOARD_SIZE*/][BOARD_SIZE]);

int bitNum(Cell c) { for (int i = 0; ; ++i) if (1 << (i - 1) >= c) return i; }
void printBoard(const Cell board[/*BOARD_SIZE*/][BOARD_SIZE], bool pretty)
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    if (i > 0 && i % SQUARE_SIDE == 0) {
      for (size_t j = 0; j < BOARD_SIZE; ++j) {
	if (j > 0 && j % SQUARE_SIDE == 0) printf("+-");
	printf("--");
      }
      printf("\n");
    }
    for (size_t j = 0; j < BOARD_SIZE; ++j) {
      if (j > 0 && j % SQUARE_SIDE == 0) printf("| ");
      printf("%i ", pretty ? bitNum(board[i][j]) : board[i][j]);
    }
    printf("\n");
  }
  printf("\n");
}

static bool PRINT_DEBUG = false;

static int num_iters = 0;

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

  bool valid, complete;
  Cell prev_board[BOARD_SIZE][BOARD_SIZE] = { { 0 } };
  while ((valid = solveNaked(board)) && !(complete = isComplete(board)) && changed(board, prev_board)) {
    for (size_t i = 0; i < BOARD_SIZE; ++i) {
      for (size_t j = 0; j < BOARD_SIZE; ++j) {
	Cell accum[BOARD_SIZE][BOARD_SIZE] = { { 0 } };
	for (Cell c = 1; c < UNKNOWN; c <<= 1) {
	  Cell guess[BOARD_SIZE][BOARD_SIZE];
	  copyBoard(board, guess);
	  guess[i][j] &= c;
	  if (guess[i][j] == 0) continue;
	  if (solveNaked(guess)) {
	    for (size_t i = 0; i < BOARD_SIZE; ++i) {
	      for (size_t j = 0; j < BOARD_SIZE; ++j) {
		accum[i][j] |= guess[i][j];
	      }
	    }
	  }
	}
	if (changed(accum, board)) goto break_outer_l;
      }
    }
  break_outer_l:;
  }

  printf("%s, %s result found! Took %i iterations.\n",
	 valid ? "Valid" : "Invalid", complete ? "Complete" : "Incomplete", num_iters);
  printBoard(board, complete);
}

bool solveByCells(Cell* cells[/*BOARD_SIZE*/]);
bool solveByPossibilities(Cell* cells[/*BOARD_SIZE*/]);

bool solveNaked(Cell board[/*BOARD_SIZE*/][BOARD_SIZE])
{
  typedef void (*GroupGetter)(Cell /*board*/[][BOARD_SIZE], size_t /*idx*/, Cell* /*res*/[]);
  GroupGetter groupers[] = {column, row, block, NULL};

  Cell prev_board[BOARD_SIZE][BOARD_SIZE] = { { 0 } };
  while (changed(board, prev_board)) {
    ++num_iters;
    Cell* cells[BOARD_SIZE];
    for (GroupGetter* groupCells = &groupers[0]; *groupCells != NULL; ++groupCells) {
      for (size_t group_i = 0; group_i < BOARD_SIZE; ++group_i) {
	(*groupCells)(board, group_i, cells);

	if (PRINT_DEBUG) {
	  printBoard(board, false);
	  printf("Cells: %s %zd\n",
		 *groupCells == column ? "Col" : *groupCells == row ? "Row" : "Block", group_i + 1);
	}
	if (!solveByCells(cells)) return false;

	if (PRINT_DEBUG) {
	  printBoard(board, false);
	  printf("Possibilites: %s %zd\n",
		 *groupCells == column ? "Col" : *groupCells == row ? "Row" : "Block", group_i + 1);
	}
	if (!solveByPossibilities(cells)) return false;
      }
    }
  }
  return true;
}

void copyBoard(const Cell src[/*BOARD_SIZE*/][BOARD_SIZE], Cell dest[/*BOARD_SIZE*/][BOARD_SIZE])
{
  changed(src, dest);
}

bool changed(const Cell src[/*BOARD_SIZE*/][BOARD_SIZE], Cell dest[/*BOARD_SIZE*/][BOARD_SIZE])
{
  bool res = false;
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    for (size_t j = 0; j < BOARD_SIZE; ++j) {
      if (src[i][j] != dest[i][j]) {
	res = true;
	dest[i][j] = src[i][j];
      }
    }
  }
  return res;
}

int countBits(Cell val);

bool isComplete(const Cell board[/*BOARD_SIZE*/][BOARD_SIZE])
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

bool solveByCells(Cell* cells[/*BOARD_SIZE*/])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    Cell mask = 0;
    int num_subsets = 0;

    const Cell vals = *cells[i];
    for (size_t j = i; j < BOARD_SIZE; ++j) {
      if ((*cells[j] | vals) == vals) {
	mask |= 1 << j;
	++num_subsets;
      }
    }

    const int num_vals = countBits(vals);
    if (num_vals == num_subsets) {
      bool changed = false;
      for (size_t j = 0; j < BOARD_SIZE; ++j) {
	const Cell old = *cells[j];
	*cells[j] &= ((1 << j) & mask) ? vals : ~vals & UNKNOWN;
	if (*cells[j] != old) changed = true;
      }
      if (PRINT_DEBUG && changed) printf("Set %i of cells fulfilling %i.\n", mask, vals);
    } else if (num_vals < num_subsets) {
      if (PRINT_DEBUG) printf("Set %i of cells overfulfill %i!\n", mask, vals);
      return false;
    }
  }
  return true;
}

Cell transposeBits(Cell* const cells[/*BOARD_SIZE*/], size_t bit_i);

bool solveByPossibilities(Cell* cells[/*BOARD_SIZE*/])
{
  for (size_t i = 0; i < BOARD_SIZE; ++i) {
    Cell mask = 0;
    int num_subsets = 0;

    const Cell vals = transposeBits(cells, i);
    for (size_t j = i; j < BOARD_SIZE; ++j) {
      if ((transposeBits(cells, j) | vals) == vals) {
	mask |= 1 << j;
	++num_subsets;
      }
    }

    const int num_vals = countBits(vals);
    if (num_vals == num_subsets) {
      bool changed = false;
      for (size_t j = 0; j < BOARD_SIZE; ++j) {
	const Cell old = *cells[j];
        *cells[j] &= ((1 << j) & vals) ? mask : ~mask & UNKNOWN;
	if (*cells[j] != old) changed = true;
      }
      if (PRINT_DEBUG && changed) printf("Set %i of possibilities fulfilled by %i.\n", vals, mask);
    } else if (num_vals < num_subsets) {
      if (PRINT_DEBUG) printf("Set %i of possibilities cannot be fulfilled by %i!\n", vals, mask);
      return false;
    }
  }
  return true;
}

int countBits(Cell val)
{
  int res = 0;
  for (size_t i = 0; i < BOARD_SIZE; ++i) if (val & (1 << i)) ++res;
  return res;
}

Cell transposeBits(Cell* const cells[/*BOARD_SIZE*/], size_t bit_i)
{
  Cell res = 0;
  for (size_t i = 0; i < BOARD_SIZE; ++i) if (*cells[i] & (1 << bit_i)) res |= 1 << i;
  return res;
}
