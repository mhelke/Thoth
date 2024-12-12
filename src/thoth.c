/*
    Thoth Engine
    © Matthew Helke 2024
*/

#include "bitboard.h"
#include "board.h"
#include "perft.h"
#include "uci.h"
#include "move.h"
#include "search.h"

#define test_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "

int main() {
    // Initialize move tables
    init_tables();

    int debug = 0;
    if (debug) {
        load_fen(test_position);
        Moves move_list[1];
        generate_moves(move_list);
        print_board();

        search(7);

        // perft_tests();

        return 0;
    }
    uci_main();
}