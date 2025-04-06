/*
    Thoth Engine
    © Matthew Helke 2025
*/
#include <stdio.h>

#include "bitboard.h"
#include "board.h"
#include "uci.h"
#include "move.h"
#include "search.h"
#include "table.h"
#include "eval.h"

#include "perft.h"
#include "tests.h"

#define debug_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define test_arg "test"
#define debug_arg "debug"

int debug_mode(char* fen, int depth) {
    Board* board = create_board();
    load_fen(fen, board);
    print_board(board);
    printEval(board);
    search(depth, board);
    free_board(board);
    return 0;
}

int run_tests() {
    test();
    perft_tests();
    return 0;
}

void initialize() {
    init_hash_keys();
    init_evaluation_masks();
}

int main(int argc, char* argv[]) {
    initialize();
    if (argc > 1) {
        if (strcmp(argv[1], test_arg) == 0) return run_tests();
        if (strcmp(argv[1], debug_arg) == 0) {
            int depth = argc > 3 ? atoi(argv[3]) : 10;
            return debug_mode(argc > 2 ? argv[2] : debug_position, depth);
        }
    }
    uci_main();
}
