#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <windows.h> // really hate to do this

#include "../sudoku.h"

#define NUM_CONSTRAINTS 10

// for windows console handling / colour printing
HANDLE win_console_handle;
CONSOLE_SCREEN_BUFFER_INFO consoleInfo;
WORD saved_attributes;

// scuffed getline to make this shit work. fuck windows fr
// appends a null terminator. this means it reads up to maxchars of characters into the buffer, and then appends a nullterm, so the buffer has to be maxchars + 1 bytes in size
int scuffed_getline(char *buf, unsigned int maxchars) {
    if (buf == NULL) return 0;
    int i = 0;
    char a = getc(stdin);

    while (a != '\n') {
        if (i <= maxchars) {
            buf[i] = a;
            i++;
        }
        a = getc(stdin);
    }

    buf[i] = '\0';
    return i;
}

// acquire console handle
void initialise_con_handler(void) {
    win_console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

    // save current attributes
    GetConsoleScreenBufferInfo(win_console_handle, &consoleInfo);
    saved_attributes = consoleInfo.wAttributes;
}

// yellow background, black foreground
void set_render_yellowbg_blackfg() {
    SetConsoleTextAttribute(win_console_handle, BACKGROUND_GREEN | BACKGROUND_RED);
    //printf("\033[30;43m");
}

// black backgroung, white foreground
void set_render_blackbg_whitefg() {
    SetConsoleTextAttribute(win_console_handle, FOREGROUND_GREEN | FOREGROUND_RED | FOREGROUND_BLUE);
    // printf("\033[0m");
}

void reset_render_bg_fg() {
    SetConsoleTextAttribute(win_console_handle, saved_attributes);
}

void sudoku_render_board(sudoku_board *board) {
    uint8_t box_size = sudoku_get_box_len();
    printf("  ");
    for (uint8_t r = 1; r <= SUDOKU_SIZE; r++) {
        printf("%d", r); // TODO: for larger r this should produce an alphabetic value
        if (r % box_size == 0 && r != SUDOKU_SIZE) {
            printf(" ");
        }
    }

    printf("\n\n");
    uint8_t ent;
    for (uint8_t i = 0; i < SUDOKU_SIZE; i++) {
        printf("%d ", i + 1); // TODO: for larger i + 1 this should produce an alphabetic value
        for (uint8_t j = 0; j < SUDOKU_SIZE; j++) {
            if (board->tiles[i][j].definite_val != 0) {
                printf("%d", board->tiles[i][j].definite_val);
            }
            else {
                ent = sudoku_get_entropy(board, i, j);
                if (ent) {
                    set_render_yellowbg_blackfg();
                    printf("%d", ent);
                    set_render_blackbg_whitefg();
                }
                else {
                    printf(".");
                }
            }
            if ((j + 1) % box_size == 0 && (j + 1) != SUDOKU_SIZE) {
                printf(" ");
            }
        }

        printf("\n");
        if ((i + 1) % box_size == 0 && (i + 1) != SUDOKU_SIZE) {
            printf("\n");
        }
    }
}

void game_initialise(void) {
    // initialise the windows console handling also save attributes to restore later
    initialise_con_handler();

    // display title and print build date
    printf("--- Sudoku Game ---\n");
    printf("Compiled at %s on %s\n\n", __TIME__, __DATE__);

    // create new board
    board = new_sudoku_board();
    if (board == NULL) {
        printf("Error: cannot create sudoku board (ALLOCATION_FAILURE)\n");
        return EXIT_FAILURE;
    }
}

void game_exit(void) {
    reset_render_bg_fg();
    free(board);
}

sudoku_board *board;

int main(int argc, char const *argv[]) {
    uint8_t x;
    uint8_t y;
    uint8_t val;
    int collapse_retval;

    game_initialise();

    // add constraints to the board
    for (size_t i = 0; i < NUM_CONSTRAINTS; i++) {
        sudoku_add_random_constraint(board);
    }

    printf("HOW TO PLAY: Write your input as \"<x><y> <value>\" (for example: \"12 3\") or type \"quit\" to quit.\nYellow cells display the number of possible values inside them.\n\n");

    char *prompt = (char*) malloc(5);
    char single_char = 0;
    while (1) {
        sudoku_render_board(board);
        printf("> ");
        scuffed_getline(prompt, 4); // get a line of input

        if (sscanf(prompt, "%1hhu%1hhu %1hhu", &x, &y, &val) == 3) {
            collapse_retval = sudoku_collapse_tile(board, (sudoku_coord) {x - 1, y - 1}, val);

            if (collapse_retval == OUT_OF_RANGE) printf("Value %d is not allowed\n", val);
            else if (collapse_retval == WRONG_VALUE) printf("Impossible value (%d) for tile at X: %d, Y: %d\n", val, x, y);
            else if (collapse_retval == CONST_POS) printf("Tile at X: %d, Y: %d cannot be changed because it is a constant tile\n", x, y);
        }
        else if (strcmp(prompt, strlwr("quit")) == 0) { // case-insensitive check for "quit"
            break;
        }
        else {
            printf("Invalid input: please write your input as \"<x><y> <value>\" or type \"quit\" to quit\n");
        }
    }

    printf("Goodbye.\n");
    free(prompt);
    game_exit();

    return EXIT_SUCCESS;
}
