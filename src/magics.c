#include <string.h>
#include <stdio.h>

#include "magics.h"
/** 
* This approach attempts to find magic numbers by trying random numbers with a low number of non-zero bits by brute force.
* This code is not run with the engine, but was used to generate the magic numbers seen in [bitboard.c].

*************************************************************************
* Credit to Tord Romstad for this approach 
* https://www.chessprogramming.org/Looking_for_Magics#Feeding_in_Randoms
*************************************************************************
*/

unsigned int state = 1804289383; // random starting number

unsigned int generate_random_U32_number() {
    unsigned int num = state;

    // XOR shift 32 algorithm
    num ^= num << 13;
    num ^= num >> 17;
    num ^= num << 5;

    // update state
    state = num;
    return num;
}

 Bitboard generate_random_U64_number() {

    // Random numbers
    Bitboard n1, n2, n3, n4;
    
    // init random numbers slicing 16 bits from MS1B side
    n1 = (Bitboard)(generate_random_U32_number() & 0xFFFF);
    n2 = (Bitboard)(generate_random_U32_number() & 0xFFFF);
    n3 = (Bitboard)(generate_random_U32_number() & 0xFFFF);
    n4 = (Bitboard)(generate_random_U32_number() & 0xFFFF);
    
    // Shuffle
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
 }

 Bitboard generate_magic_bitboard() {
    return generate_random_U64_number() & generate_random_U64_number() & generate_random_U64_number();
 }

Bitboard _find_magic_bitboard(int square, int relevant_bits, int is_bishop) {
    Bitboard occupancies[4096];
    Bitboard attacks_table[4096];
    Bitboard used_attacks[4096]; 

    Bitboard attack_mask = is_bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    int occupancy_indicies = 1 << relevant_bits;

    for (int i = 0; i < occupancy_indicies; i++) {
        occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);
        attacks_table[i] = is_bishop ? generate_bishop_attacks(square, occupancies[i]) : generate_rook_attacks(square, occupancies[i]);
    }

    // test generated magic bitboard
    for (int i = 0; i < 100000000; i++) {

        // Random magic bitboard candidate
        Bitboard magic_bitboard = generate_magic_bitboard();

        // Skip known bad magic numbers
        if (count_bits((attack_mask * magic_bitboard) & 0xFF00000000000000) < 6) {
            continue;
        }

        // Save to used attacks
        memset(used_attacks, 0ULL, sizeof(used_attacks));
        
        int index, fail;

        // test magic bitboard
        for (index = 0, fail = 0; !fail && index < occupancy_indicies; index++) {
            int magic_index = (int)((occupancies[index] * magic_bitboard) >> (64 - relevant_bits));

            // Magic index works, not used
            if (used_attacks[magic_index] == 0ULL) {
                used_attacks[magic_index] = attacks_table[index];
            } else if (used_attacks[magic_index] != attacks_table[index]) {
                // Magic index doesn't work, fail
                fail = 1;
            }
        }

        if (fail) {
            continue;
        }

        return magic_bitboard;
    }
        // Magic bitboard doesn't work
        printf("Magic bitboard failed");
        return 0ULL;
}

Bitboard _rook_magics[64];
Bitboard _bishop_magics[64];


void _init_magic_bitboards() {
    // Loops over board and check magic bitboard for each square
    // Rook
    for (int square = 0; square < 64; square++) {
        _rook_magics[square] = _find_magic_bitboard(square, rook_relevant_bits[square], sROOK);
    }

    // Bishop
    for (int square = 0; square < 64; square++) {
        // Bishop
        _bishop_magics[square] = _find_magic_bitboard(square, bishop_relevant_bits[square], sBISHOP);
    }
}