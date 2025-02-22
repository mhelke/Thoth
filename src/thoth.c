/*
    Thoth Engine
    © Matthew Helke 2024
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

#define test_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define mate_position "8/8/8/8/8/K7/Q7/7k w - 0 1 "

int debug_mode() {
    Board* board = create_board();
    load_fen(test_position, board);
    print_board(board);
    printEval(board);
    search(10, board);
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

int main() {
    initialize();
    int debug = 0;
    int tests = 0;
    if (tests) return run_tests();
    if (debug) return debug_mode();
    uci_main();
}