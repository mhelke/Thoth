/*
Universal Chess Interface (UCI) is the standard communication protocol which GUIs use to communicate with chess engines.
More info on UCI: https://www.chessprogramming.org/UCI
*/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "move.h"
#include "board.h"

// Parse move string from UCI
// Example move from UCI protocol: "e7e8q"
int parse_move(const char *str) {
    Moves move_list[1];
    generate_moves(move_list);

    int src = (str[0] - 'a') + (8 - (str[1] - '0')) * 8;
    int target = (str[2] - 'a') + (8 - (str[3] - '0')) * 8;

    for (int i = 0; i < move_list->count; i++) {
        int move = move_list->moves[i];
        if (MOVE_SRC(move) == src && MOVE_TARGET(move) == target) {
            int promoted_piece = MOVE_PROMOTED(move);
            if (promoted_piece) {
                char promotion_char = str[4];
                if ((promoted_piece == Q || promoted_piece == q) && promotion_char == 'q') return move;
                if ((promoted_piece == R || promoted_piece == r) && promotion_char == 'r') return move;
                if ((promoted_piece == B || promoted_piece == b) && promotion_char == 'b') return move;
                if ((promoted_piece == N || promoted_piece == n) && promotion_char == 'n') return move;
                continue;
            }
            return move;
        }
    }
    return 0;
}

// Parse position command from UCI
// Example commands: `position startpos`, `position fen <FEN_STRING>`
// The command can also include moves: `position startpos moves e2e4 e7e5`
void parse_position(char *command) {
    command += 9; // shift pointer to position argument
    char *current = command;

    // Parse startpos command and init board to start position
    if (strncmp(command, "startpos", 8) == 0) {
        load_fen(start_position);
    } else {
        current = strstr(command, "fen");

        if (current == NULL) {
            load_fen(start_position);
        } else {
            current += 4; // shift pointer to start of FEN string
            load_fen(current);
        }
    }

    current = strstr(command, "moves");

    // Parse moves
    if (current != NULL) {
         current += 6; // shift pointer to start of given moves token
         while (*current) {
            int move = parse_move(current);
            if (move == 0) {
                break;
            }
            make_move(move, ALL_MOVES);

            // Move pointer to next move
            while (*current && *current != ' ') {
                current++;   
            }
            current++;
         }
    }
    print_board(); // remove
}

// TODO: Placeholder for testing UCI. Move when search is implemented.
void search_position(int depth) {
    printf("bestmove e7e5\n");
}

// Parse the go command from UCI
// Example command: go depth 6
void parse_go(char * command) {
    int depth = -1;
    char *current = NULL;
    if (strstr(command, "depth")) {
        depth = atoi(current + 6); // shift pointer to start of depth
    } else {
        depth = 5; // TODO: placeholder
    }
    search_position(depth);
}

void set_info() {
    printf("id name Thoth\n");
    printf("id author Matthew Helke\n");
    printf("uciok\n");
}

void handle_uci_ready() {
    printf("readyok\n");
}

int parse_line() {
    fflush(stdout); // needed?
    char input[2000];
    memset(input, 0, sizeof(input));
    if (fgets(input, 2000, stdin) == NULL) {
        return 0;
    }
    if (input[0] == '\n') {
        return 0;
    }
    if (strncmp(input, "isready", 7) == 0) {
        handle_uci_ready();
        return 1;
    }
    if (strncmp(input, "position", 8) == 0) {
        parse_position(input);
        return 1;
    }
    if (strncmp(input, "ucinewgame", 10) == 0) {
        parse_position("position startpos");
        return 1;
    }
    if (strncmp(input, "go", 2) == 0) {
        parse_go("input");
        return 1;
    }
    if (strncmp(input, "quit", 4) == 0) {
        return 0;
    }
    if (strncmp(input, "uci", 3) == 0) {
        set_info();
        return 1;
    }
}

void uci_main() {
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);
    while (parse_line());
}