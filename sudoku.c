#include <math.h>
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint-gcc.h>


#include "sudoku.h"

sudoku_board *new_sudoku_board(void) {
    sudoku_board *board = (sudoku_board *) malloc(sizeof(sudoku_board));
    if (board == NULL) {
        return NULL;
    }

    for (sudoku_num_t i = 0; i < SUDOKU_SIZE; i++) {
        for (sudoku_num_t j = 0; j < SUDOKU_SIZE; j++) {
            board->tiles[i][j].definite_val = 0;
            board->tiles[i][j].constraint = false;

            for (sudoku_num_t p = 0; p < SUDOKU_SIZE; p++) {
                board->tiles[i][j].superpos[p] = true;
            }
        }
    }

    return board;
}

sudoku_num_t sudoku_get_box_len(void) {
    return sqrt(SUDOKU_SIZE);
}

sudoku_num_t sudoku_get_box_i(sudoku_num_t tile_pos) {
    return tile_pos / sudoku_get_box_len();
}


sudoku_num_t sudoku_get_entropy(sudoku_board *board, sudoku_num_t i, sudoku_num_t j) {
    sudoku_num_t entropy = 0;
    if (board->tiles[i][j].definite_val == 0) {
        for (sudoku_num_t p = 0; p < SUDOKU_SIZE; p++) {
            if (board->tiles[i][j].superpos[p]) entropy++;
        }
    }

    return entropy;
}
/*
sudoku_coord sudoku_blame_exclusion(sudoku_board *board, sudoku_num_t i, sudoku_num_t j, sudoku_num_t excluded) {
    if (board == NULL) return;

    
}
*/

// collapses a tile that doesnt have a real "definite" value, using a single "stable" superposition, by finding out if there is one and collapsing down to it
void sudoku_collapse_from_stable_superposition_to_definite(sudoku_board *board, sudoku_num_t i, sudoku_num_t j) {
    if (board->tiles[i][j].definite_val == 0) {
        for (sudoku_num_t p = 0; p < SUDOKU_SIZE; p++) {
            if (board->tiles[i][j].superpos[p]) {
                board->tiles[i][j].definite_val = p + 1;
                break;
            }
        }
    }
}


void sudoku_uncollapse_tile(sudoku_board *board, sudoku_coord coord) {
    if (board == NULL) return;
    sudoku_num_t current_tile_value = board->tiles[coord.x][coord.y].definite_val;

    sudoku_num_t box_size = sudoku_get_box_len();
    sudoku_num_t x_box_offset = sudoku_get_box_i(coord.x);
    sudoku_num_t y_box_offset = sudoku_get_box_i(coord.y);
    

    if (current_tile_value != 0) {
        if (board->tiles[coord.x][coord.y].constraint == true) {
            return;
        }

        board->tiles[coord.x][coord.y].definite_val = 0;

        for (sudoku_num_t i = 0; i < box_size; i++) {
            for (sudoku_num_t j = 0; j < box_size; j++) {
                if (sudoku_is_val_allowed_for_tile(board, (sudoku_coord) {(x_box_offset * box_size) + i, (y_box_offset * box_size) + j}, current_tile_value)) {
                    board->tiles[(x_box_offset * box_size) + i][(y_box_offset * box_size) + j].superpos[current_tile_value - 1] = true;
                }
            }
        }

        for (sudoku_num_t i = 0; i < SUDOKU_SIZE; i++) {
            if (sudoku_is_val_allowed_for_tile(board, (sudoku_coord) {i, coord.y}, current_tile_value)) {
                board->tiles[i][coord.y].superpos[current_tile_value - 1] = true;
            }
        }

        for (sudoku_num_t j = 0; j < SUDOKU_SIZE; j++) {
            if (sudoku_is_val_allowed_for_tile(board, (sudoku_coord) {coord.x, j}, current_tile_value)) {
                board->tiles[coord.x][j].superpos[current_tile_value - 1] = true;
            }
        }
    }
}

// return value of false means val isnt a valid state for the tile at coord
bool sudoku_is_val_allowed_for_tile(sudoku_board *board, sudoku_coord coord, sudoku_num_t superpos) {
    // check if the box contains a field whose definite value is equal to val
    sudoku_num_t box_size = sudoku_get_box_len();
    sudoku_num_t x_box_offset = coord.x / box_size;
    sudoku_num_t y_box_offset = coord.y / box_size;

    for (sudoku_num_t i = 0; i < box_size; i++) {
        for (sudoku_num_t j = 0; j < box_size; j++) {
            if (board->tiles[(x_box_offset * box_size) + i][(y_box_offset * box_size) + j].definite_val == superpos) {
                return false;
            }
        }
    }

    // check if either the row or the collumn contains a field whose definite value is equal to val
    for (sudoku_num_t i = 0; i < SUDOKU_SIZE; i++) {
        if (board->tiles[i][coord.y].definite_val == superpos) {
            return false;
        }
    }

    for (sudoku_num_t j = 0; j < SUDOKU_SIZE; j++) {
        if (board->tiles[coord.x][j].definite_val == superpos) {
            return false;
        }
    }

    return true;
}

collapse_return_code sudoku_collapse_tile(sudoku_board *board, sudoku_coord coord, sudoku_num_t val) {
    if (val > SUDOKU_SIZE || val == 0) return OUT_OF_RANGE;
    if (board->tiles[coord.x][coord.y].superpos[val - 1] == false) return WRONG_VALUE;

    sudoku_num_t box_size = sudoku_get_box_len();
    sudoku_num_t x_box_offset = coord.x / box_size;
    sudoku_num_t y_box_offset = coord.y / box_size;

    sudoku_num_t current_tile_value = board->tiles[coord.x][coord.y].definite_val;

    if (current_tile_value != 0) { // modifying a tile
        if (board->tiles[coord.x][coord.y].constraint == true) {
            return CONST_POS;
        }

        board->tiles[coord.x][coord.y].definite_val = 0; // quickly suspend our disbelief for a second. this is necessary to perform superpos legitimacy checking

        for (sudoku_num_t i = 0; i < box_size; i++) {
            for (sudoku_num_t j = 0; j < box_size; j++) {
                if (sudoku_is_val_allowed_for_tile(board, (sudoku_coord) {(x_box_offset * box_size) + i, (y_box_offset * box_size) + j}, current_tile_value)) {
                    board->tiles[(x_box_offset * box_size) + i][(y_box_offset * box_size) + j].superpos[current_tile_value - 1] = true;
                }
            }
        }

        for (sudoku_num_t i = 0; i < SUDOKU_SIZE; i++) {
            if (sudoku_is_val_allowed_for_tile(board, (sudoku_coord) {i, coord.y}, current_tile_value)) {
                board->tiles[i][coord.y].superpos[current_tile_value - 1] = true;
            }
        }

        for (sudoku_num_t j = 0; j < SUDOKU_SIZE; j++) {
            if (sudoku_is_val_allowed_for_tile(board, (sudoku_coord) {coord.x, j}, current_tile_value)) {
                board->tiles[coord.x][j].superpos[current_tile_value - 1] = true;
            }
        }
    }

    board->tiles[coord.x][coord.y].definite_val = val;
    printf("Tile has been collapsed\n");

    for (sudoku_num_t i = 0; i < box_size; i++) {
        for (sudoku_num_t j = 0; j < box_size; j++) {
            board->tiles[(x_box_offset * box_size) + i][(y_box_offset * box_size) + j].superpos[val - 1] = false;
            sudoku_collapse_from_stable_superposition_to_definite(board, (x_box_offset * box_size) + i, y_box_offset * box_size + j);
        }
    }

    for (sudoku_num_t i = 0; i < SUDOKU_SIZE; i++) {
        board->tiles[i][coord.y].superpos[val - 1] = false;
        sudoku_collapse_from_stable_superposition_to_definite(board, i, coord.y);
    }

    for (sudoku_num_t j = 0; j < SUDOKU_SIZE; j++) {
        board->tiles[coord.x][j].superpos[val - 1] = false;
        sudoku_collapse_from_stable_superposition_to_definite(board, coord.x, j);
    }
    return SUCCESS;
}

void sudoku_add_random_constraint(sudoku_board *board) {
    sudoku_num_t randx;
    sudoku_num_t randy;
    sudoku_num_t randval;

    srand(clock() + time(NULL));
    srand(rand() + 27); // this is so bad but it works

    while (1) {
        randx = (rand() % SUDOKU_SIZE);
        randy = (rand() % SUDOKU_SIZE);
        randval = (rand() % SUDOKU_SIZE) + 1;
        if (sudoku_collapse_tile(board, (sudoku_coord) {randx, randy}, randval) == SUCCESS) {
            board->tiles[randx][randy].constraint = true;

            return;
        }

        srand(clock() + rand());
    }
}
