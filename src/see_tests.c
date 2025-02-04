#include <stdio.h>
#include <stdlib.h>

#include "see_tests.h"
#include "bitboard.h"
#include "search.h"
#include "board.h"

typedef struct {
    char* fen;
    int expected_result;
    int to;
    int from;
} SeeTest;

#define NUM_TESTS 13

SeeTest tests[NUM_TESTS] = {
    { "1k1r4/1pp4p/p7/4p3/8/P5P1/1PP4P/2K1R3 w - - 0 1 ", 100, 28, 60 }, // Rxe5
    { "1k1r3q/1ppn3p/p4b2/4p3/8/P2N2P1/1PP1R1BP/2K1Q3 w - - 0 1 ", -225, 28, 43 }, // Nxe5
    { "1k1r3q/1ppn3p/p4b2/4P3/8/P2N2P1/1PP1R1BP/2K1Q3 b - - 0 1 ", -225, 28, 11 }, // Nxe5

    // Special case for promotion - the promoted piece value is not included
    {"8/4kp2/2npp3/1Nn5/1p2PQP1/7q/1PP1B3/4KR1r b - - 0 1;", 0, 61, 63 }, // Rxf1+
    
    // Pawn captures
    {"3r3k/3r4/2n1n3/8/1N1p4/2PR4/1B1Q4/3R3K w - - 0 1", -160, 35, 43}, // Rxd4
    { "k3r3/4q1b1/4rp2/4P1n1/8/3N1N2/4R3/K3R3 b - - 0 1", -160, 28, 20}, // Rxe5
   
    // King illegal captures
    {"r1bq1r2/pp1ppkbp/4N1pB/n3P3/8/2N5/PPP2PPP/R2QK2R w KQ - 0 1", 335, 14, 20}, // Nxg7

    // King legal capture
    {"rnq1k2r/1b3ppp/p2bpn2/1p1p4/3N4/1BN1P3/PPP2PPP/R1BQR1K1 b kq - 0 1", -235, 55, 19}, // Bxh2
    // Same position as a above but mirrored
    {"r1bqr1k1/ppp2ppp/1bn1p3/3n4/1P1P4/P2BPN2/1B3PPP/RNQ1K2R w KQkq - 0 1", -235, 15, 43}, // Bxh2
    
    {"3N4/2K5/2n5/1k6/8/8/8/8 b - -", 0, 3, 18},
    {"3n4/2k5/2N5/1K6/8/8/8/8 w - - 0 1", 0, 3, 18},
    {"r1bqkb1r/2pp1ppp/p1n5/1p2p3/3Pn3/1B3N2/PPP2PPP/RNBQ1RK1 b kq - 0 1", 0, 35, 18}, // Nxd4
    {"r1bq1r2/pp1ppkbp/4N1p1/n3P1B1/8/2N5/PPP2PPP/R2QK2R w KQ - 0 1", 10, 14, 20}, // Nxg7

};

void test_see() {
    Board* board = create_board();

    for (int i = 0; i < NUM_TESTS; i++) {
        load_fen(tests[i].fen, board);
        int val = see(board, tests[i].to, tests[i].from);

        if (val != tests[i].expected_result) {
            printf("[%d] FAILURE testing SEE: Expected %d, got %d\n", i, tests[i].expected_result, val);
            exit(EXIT_FAILURE);
        }
    }
    printf("SEE tests passed\n");
    free_board(board);
}
