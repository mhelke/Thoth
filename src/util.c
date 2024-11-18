// https://www.chessprogramming.org/Looking_for_Magics

#include "bitboard.h"

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
    n1 = (Bitboard)(generate_random_U32_number()) & 0xFFFF;
    n2 = (Bitboard)(generate_random_U32_number()) & 0xFFFF;
    n3 = (Bitboard)(generate_random_U32_number()) & 0xFFFF;
    n4 = (Bitboard)(generate_random_U32_number()) & 0xFFFF;
    
    // Shuffle
    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
 }

