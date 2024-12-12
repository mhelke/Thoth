/************************************************************************
* Universal Chess Interface (UCI) is the standard communication protocol 
* which GUIs use to communicate with chess engines.
* More info on UCI: https://www.chessprogramming.org/UCI
*
* Credit for handling time controls:
*   Vice Chess Engine by Richard Allbert
*   https://github.com/bluefeversoft/vice
************************************************************************/

#if defined(_WIN32) || defined(_WIN64)
    #include <io.h>
    #include <windows.h>
#else
    #include <unistd.h>
    #include <sys/select.h>
#endif
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "move.h"
#include "board.h"
#include "search.h"
#include "uci.h"
#include "util.h"

// Time control variables
int quit = 0;
int movestogo = 30;
int movetime = -1;
int time = -1;
int inc = 0;
int starttime = 0;
int stoptime = 0;
int timeset = 0;
int stopped = 0;

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

// Parse the go command from UCI
void parse_go(char *command) {
    int depth = -1;
    char *current = NULL;
    if ((current = strstr(command,"infinite"))) { /* Let search continue until stopped by user */}

    // Black increment
    if ((current = strstr(command,"binc")) && side == BLACK) {
        inc = atoi(current + 5);
    }

    // White increment
    if ((current = strstr(command,"winc")) && side == WHITE) {
        inc = atoi(current + 5);
    }

    // White time limit
    if ((current = strstr(command,"wtime")) && side == WHITE) {
        time = atoi(current + 6);
    }
    // Black time limit
    if ((current = strstr(command,"btime")) && side == BLACK) {
        time = atoi(current + 6);
    }

    // Number of moves to go until bonus time
    if ((current = strstr(command,"movestogo"))) {
        movestogo = atoi(current + 10);
    }

    // Time per move
    if ((current = strstr(command,"movetime"))) {
        movetime = atoi(current + 9);
    }

    // Fixed depth search
    if ((current = strstr(command, "depth"))) {
        depth = atoi(current + 6); // shift pointer to start of depth
    }

    // Handle fixed time per move
    if(movetime != -1) {
        time = movetime;
        movestogo = 1;
    }

    starttime = get_ms();

    // Handle time control
    if(time != -1) {
        timeset = 1;
        time /= movestogo;
        
        // Provide buffer
        if (time > 1500) time -= 50;
        
        // How long the engine has to calculate based on the time control
        stoptime = starttime + time + inc;
    }

    // Set a large default depth if not specified and rely on stoptime to stop the search
    if(depth == -1) {
        depth = 64;
    }

    printf("time: %d start: %d stop: %d depth: %d timeset: %d\n",
    time, starttime, stoptime, depth, timeset);
    stopped = 0;
    search(depth);
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
    fflush(stdout);
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
        parse_go(input);
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

/************************************************
* Credit to Vice Chess Engine by Richard Allbert
************************************************/
int input_waiting() {
    #ifdef _WIN32
        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init) {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe) {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT | ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }

        if (pipe) {
            if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
            return dw;
        } else {
            GetNumberOfConsoleInputEvents(inh, &dw);
            return dw <= 1 ? 0 : dw;
        }
    #else
        fd_set readfds;
        struct timeval tv;
        FD_ZERO(&readfds);
        FD_SET(fileno(stdin), &readfds);
        tv.tv_sec = 0; 
        tv.tv_usec = 0;
        select(16, &readfds, NULL, NULL, &tv);
        return (FD_ISSET(fileno(stdin), &readfds));
    #endif
}

/************************************************
* Credit to Vice Chess Engine by Richard Allbert
************************************************/
void read_input() {
    int bytes;
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting()) {
        stopped = 1;
        
        do {
            bytes=read(fileno(stdin), input, 256);
        } while (bytes < 0);

        endc = strchr(input,'\n');
        if (endc) *endc=0;
        
        if (strlen(input) > 0) {
            // Listen for quit or stop command and terminate search
            if (!strncmp(input, "quit", 4)) {
                quit = 1;
            } else if (!strncmp(input, "stop", 4)) {
                quit = 1;
            }
        }   
    }
}

int should_stop() {
    // Time is up, stop search
    if(timeset == 1 && get_ms() > stoptime) {
		stopped = 1;
	}
    // Check if engine was stopped
	read_input();
    return stopped;
}