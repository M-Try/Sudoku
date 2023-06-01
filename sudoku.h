#pragma once

#include <stdbool.h>
#include <stdint-gcc.h>

// defines the size of the board, the amount of tiles per box, the amount of states per tile
// due to how sudoku works this is necessarily a number whose prime factorization is equal to its root squared
#ifndef SUDOKU_SIZE
#define SUDOKU_SIZE 9
#endif

typedef uint8_t sudoku_num_t;

typedef struct {
    sudoku_num_t x;
    sudoku_num_t y;
} sudoku_coord;

typedef struct {
    sudoku_num_t definite_val;
    bool superpos[SUDOKU_SIZE];
    bool constraint;
} sudoku_tile;

typedef struct {
    sudoku_tile tiles[SUDOKU_SIZE][SUDOKU_SIZE];
} sudoku_board;


sudoku_board *new_sudoku_board(void);
sudoku_num_t sudoku_get_box_len(void);
sudoku_num_t sudoku_get_box_i(sudoku_num_t tile_pos);

void sudoku_collapse_from_stable_superposition_to_definite(sudoku_board *board, sudoku_num_t i, sudoku_num_t j);
// return value of false means val isnt a valid state for the tile at coord
bool sudoku_is_val_allowed_for_tile(sudoku_board *board, sudoku_coord coord, sudoku_num_t superpos);

typedef enum {
    SUCCESS,
    OUT_OF_RANGE,
    WRONG_VALUE,
    CONST_POS
} collapse_return_code;

sudoku_num_t sudoku_get_entropy(sudoku_board *board, sudoku_num_t i, sudoku_num_t j);
collapse_return_code sudoku_collapse_tile(sudoku_board *board, sudoku_coord coord, sudoku_num_t val);
void sudoku_uncollapse_tile(sudoku_board *board, sudoku_coord coord);
void sudoku_add_random_constraint(sudoku_board *board);
