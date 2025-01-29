#include <stdio.h>
#include <string.h>

#include "board.h"
#include "move.h"
#include "util.h"
#include "bitboard.h"
#include "table.h"

Bitboard nodes;

void perft(int depth, Board* board) {
    if (depth == 0) {
        nodes++;
        return;
    }

    Moves move_list[1];
    generate_moves(move_list, board);

    for (int move_count = 0; move_count < move_list->count; move_count++) {   
        COPY_BOARD(board);
        if (!make_move(move_list->moves[move_count], board)) {
            continue;
        }
        perft(depth - 1, board);
        UNDO(board);        
    }
}

int perft_test(char *fen, int depth, const unsigned long long *expected_values, Board* board) {
    load_fen(fen, board);
    nodes = 0;

    long start = get_ms();

    Moves move_list[1];
    generate_moves(move_list, board);

    for (int move_count = 0; move_count < move_list->count; move_count++) {
        COPY_BOARD(board);
        if (make_move(move_list->moves[move_count], board)) {
            perft(depth - 1, board);
            UNDO(board);

            // Debug hash key generation
            /*
            Bitboard expected_hash = generate_hash_key(board);
            if (board->hash_key != expected_hash) {
                printf("Move: ");
                print_move(move_list->moves[move_count]);
                print_board(board);
                printf("Expected hash key: %llx\n", expected_hash);
                getchar();
            }
            */
        }
    }
    long end = get_ms();

    printf("Depth: %d\n", depth);
    printf("Nodes: %llu\n", nodes);
    printf("Time: %ld ms\n", end - start);

    if (expected_values) {
        if (nodes == expected_values[depth - 1]) {
            printf("Validation passed: %llu nodes\n", nodes);
            return 1;
        } else {
            printf("Validation failed: expected %llu nodes, but got %llu nodes\n", expected_values[depth - 1], nodes);
            return 0;
        }
    }
}


void perft_tests() {
    struct Test {
        char *fen;
        const unsigned long long *expected_values;
        int max_depth;
    };

    struct Test tests[] = {
        {
            "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
            (unsigned long long[]){20, 400, 8902, 197281, 4865609, 119060324},
            6
        },
        {
            "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -",
            (unsigned long long[]){48, 2039, 97862, 4085603, 193690690, /*8031647685*/},
            5
        },
        {
            "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -",
            (unsigned long long[]){14, 191, 2812, 43238, 674624, 11030083},
            6
        },
        {
            "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
            (unsigned long long[]){6, 264, 9467, 422333, 15833292, 706045033},
            6
        },
        {
            "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8",
            (unsigned long long[]){44, 1486, 62379, 2103487, 89941194},
            5
        }
    };

    int num_tests = sizeof(tests) / sizeof(tests[0]);

    for (int i = 0; i < num_tests; i++) {
        printf("\n\n-----Running test %d-----\n", i + 1);
        for (int depth = 1; depth <= tests[i].max_depth; depth++) {
            printf("\n[%d] Running depth %d...\n", i + 1, depth);
            Board* board = create_board();
            int passed = perft_test(tests[i].fen, depth, tests[i].expected_values, board);
            if (!passed) {
                printf("\nFAILURE for test [%d] at depth %d", i + 1, depth);
                printf("\n!! Tests failed !!\n");
                free_board(board);
                return;
            }
            free_board(board);
        }
    }
    printf("\n All tests passed!\n");        
}

