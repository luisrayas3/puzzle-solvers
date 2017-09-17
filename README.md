# Puzzle Solvers

Algorithms for solving puzzles. Just for fun.

## Sudoku

```
$ cd sudoku
```

### Build

```
$ gcc -std=c11 -Wall -o sudoku sudoku.c
```

By default the executable is built to solve 9x9 boards (SQUARE_SIDE == 3). But
this may be changed by modifying the value of SQUARE_SIDE near the top of the
source file.

### Run

```
$ ./sudoku [ - | -d ] $BOARD
```

`d` -> if included, the program will output the solving steps (e.g. for debug).

`$BOARD` must be a string of BOARD_SIZE * BOARD_SIZE (e.g. 81 for a standard
board) characters representing the starting value of each cell, or '`0`' if the
cell begins empty and needs to be solved, starting at the top-left going across
then down. For example, a 4x4 puzzle with all but the 4th row, 3rd column cell
solved would be provided:

```
$ ./sudoku -d 1234432121433402
```

Solving a 9x9 example from the internet:

```
$ ./sudoku 010200500500009060000015007003070001001000200900040300400780000090500008002003040
```