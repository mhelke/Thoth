#include <stdio.h>
#include "bitboard.h"

const Bitboard NOT_A_FILE = 18374403900871474942ULL;
const Bitboard NOT_H_FILE = 9187201950435737471ULL;
const Bitboard NOT_HG_FILE = 4557430888798830399ULL;
const Bitboard NOT_AB_FILE = 18229723555195321596ULL;

const Bitboard rook_magics[64] = {
    0x8a80104000800020ULL, 0x140002000100040ULL, 0x2801880a0017001ULL,
    0x100081001000420ULL, 0x200020010080420ULL, 0x3001c0002010008ULL,
    0x8480008002000100ULL, 0x2080088004402900ULL, 0x800098204000ULL,
    0x2024401000200040ULL, 0x100802000801000ULL, 0x120800800801000ULL,
    0x208808088000400ULL, 0x2802200800400ULL, 0x2200800100020080ULL, 
    0x801000060821100ULL,0x80044006422000ULL, 0x100808020004000ULL,
    0x12108a0010204200ULL, 0x140848010000802ULL, 0x481828014002800ULL,
    0x8094004002004100ULL, 0x4010040010010802ULL, 0x20008806104ULL,
    0x100400080208000ULL, 0x2040002120081000ULL, 0x21200680100081ULL, 
    0x20100080080080ULL, 0x2000a00200410ULL, 0x20080800400ULL,
    0x80088400100102ULL, 0x80004600042881ULL, 0x4040008040800020ULL,
    0x440003000200801ULL, 0x4200011004500ULL, 0x188020010100100ULL,
    0x14800401802800ULL, 0x2080040080800200ULL, 0x124080204001001ULL,
    0x200046502000484ULL, 0x480400080088020ULL, 0x1000422010034000ULL,
    0x30200100110040ULL, 0x100021010009ULL, 0x2002080100110004ULL,
    0x202008004008002ULL, 0x20020004010100ULL, 0x2048440040820001ULL,
    0x101002200408200ULL, 0x40802000401080ULL, 0x4008142004410100ULL,
    0x2060820c0120200ULL, 0x1001004080100ULL, 0x20c020080040080ULL,
    0x2935610830022400ULL, 0x44440041009200ULL, 0x280001040802101ULL,
    0x2100190040002085ULL, 0x80c0084100102001ULL, 0x4024081001000421ULL,
    0x20030a0244872ULL, 0x12001008414402ULL, 0x2006104900a0804ULL,
    0x1004081002402ULL,
};

const Bitboard bishop_magics[64] = {
    0x40040844404084ULL, 0x2004208a004208ULL, 0x10190041080202ULL,
    0x108060845042010ULL, 0x581104180800210ULL, 0x2112080446200010ULL,
    0x1080820820060210ULL, 0x3c0808410220200ULL, 0x4050404440404ULL,
    0x21001420088ULL, 0x24d0080801082102ULL, 0x1020a0a020400ULL,
    0x40308200402ULL, 0x4011002100800ULL, 0x401484104104005ULL,
    0x801010402020200ULL, 0x400210c3880100ULL, 0x404022024108200ULL,
    0x810018200204102ULL, 0x4002801a02003ULL, 0x85040820080400ULL,
    0x810102c808880400ULL, 0xe900410884800ULL, 0x8002020480840102ULL,
    0x220200865090201ULL, 0x2010100a02021202ULL, 0x152048408022401ULL,
    0x20080002081110ULL, 0x4001001021004000ULL, 0x800040400a011002ULL,
    0xe4004081011002ULL, 0x1c004001012080ULL, 0x8004200962a00220ULL,
    0x8422100208500202ULL, 0x2000402200300c08ULL, 0x8646020080080080ULL,
    0x80020a0200100808ULL, 0x2010004880111000ULL, 0x623000a080011400ULL,
    0x42008c0340209202ULL, 0x209188240001000ULL, 0x400408a884001800ULL,
    0x110400a6080400ULL, 0x1840060a44020800ULL, 0x90080104000041ULL,
    0x201011000808101ULL, 0x1a2208080504f080ULL, 0x8012020600211212ULL,
    0x500861011240000ULL, 0x180806108200800ULL, 0x4000020e01040044ULL,
    0x300000261044000aULL, 0x802241102020002ULL, 0x20906061210001ULL,
    0x5a84841004010310ULL, 0x4010801011c04ULL, 0xa010109502200ULL,
    0x4a02012000ULL, 0x500201010098b028ULL, 0x8040002811040900ULL,
    0x28000010020204ULL, 0x6000020202d0240ULL, 0x8918844842082200ULL,
    0x4010011029020020ULL
};

// The number of relevant occupancy squares within the path of the bishop's attack mask.
// See [gen.c] for how this was calculated.
const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 9, 9, 7, 5, 5,
    5, 5, 7, 7, 7, 7, 5, 5,
    5, 5, 5, 5, 5, 5, 5, 5,
    6, 5, 5, 5, 5, 5, 5, 6,
};

// The number of relevant occupancy squares within the path of the rook's attack mask.
// See [gen.c] for how this was calculated.
const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    11, 10, 10, 10, 10, 10, 10, 11,
    12, 11, 11, 11, 11, 11, 11, 12,
};

char ascii_pieces[12] = "PNBRQKpnbrqk";

// convert ASCII character pieces to encoded constants
int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

// Piece bitboards
Bitboard bitboards[12];

// Occupancy bitboards (white, black, all)
Bitboard occupancies[2];

// Side to move
int side = -1;

// En passant square 
int enpassant = na;

// Castling rights
int castle = 0;

// Pawn attacks table
Bitboard pawn_attacks[2][64];

// Knight attacks table
Bitboard knight_attacks[64];

// King attacks table
Bitboard king_attacks[64];

// Bishop attacks table
Bitboard bishop_attacks[64][512];

// Rook attacks table
Bitboard rook_attacks[64][4096];

// Bishop attack masks
Bitboard bishop_masks[64];

// Rook attack masks
Bitboard rook_masks[64];

// Pawn attacks
Bitboard mask_pawn_attacks(int side, int square) {
    // Attack bitboard
    Bitboard attacks = 0ULL;

    // Piece bitboard
    Bitboard bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // white = 0, black = 1
    // pawn captures offset of 7 & 9
    if (!side) {
        // Pawns on a-file can only capture on the b-file 
        if((bitboard >> 7) & NOT_A_FILE) {
            attacks |= (bitboard >> 7);
        }
        // Pawns on h-file can only capture on the g-file
        if((bitboard >> 9) & NOT_H_FILE) {
            attacks |= (bitboard >> 9);
        }
    } else {
        // Pawns on a-file can only capture on the b-file 
        if((bitboard << 7) & NOT_H_FILE) {
            attacks |= (bitboard << 7);
        }
        // Pawns on h-file can only capture on the g-file
        if((bitboard << 9) & NOT_A_FILE) {
            attacks |= (bitboard << 9);
        }
    }
    return attacks;
}

// Knight attacks
Bitboard mask_knight_attacks(int square) {
    // Attack bitboard
    Bitboard attacks = 0ULL;

    // Piece bitboard
    Bitboard bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // Generate kngiht moves.
    // Offsets 6, 10, 15, 17
    // Knights on A or B file should not jump to the H or G file 
    if ((bitboard >> 17) & NOT_H_FILE) {
        attacks |= (bitboard >> 17);
    }
    if ((bitboard >> 10) & NOT_HG_FILE) {
        attacks |= (bitboard >> 10);
    }
    // Knights on G or H file should not jump to the A or B
    if ((bitboard >> 15) & NOT_A_FILE) {
        attacks |= (bitboard >> 15);
    }
    if ((bitboard >> 6) & NOT_AB_FILE) {
        attacks |= (bitboard >> 6);
    }

    // Other direction
    if ((bitboard << 17) & NOT_A_FILE) {
        attacks |= (bitboard << 17);
    }
    if ((bitboard << 10) & NOT_AB_FILE) {
        attacks |= (bitboard << 10);
    }
    if ((bitboard << 15) & NOT_H_FILE) {
        attacks |= (bitboard << 15);
    }
    if ((bitboard << 6) & NOT_HG_FILE) {
        attacks |= (bitboard << 6);
    }
    return attacks;
}

// King attacks
Bitboard mask_king_attacks(int square) {
    // Attack bitboard
    Bitboard attacks = 0ULL;

    // Piece bitboard
    Bitboard bitboard = 0ULL;

    // Set piece on board
    set_bit(bitboard, square);

    // Generate king moves.
    if (bitboard >> 8) {
        attacks |= (bitboard >> 8);   
    }
    if (bitboard >> 9 & NOT_H_FILE) {
        attacks |= (bitboard >> 9);   
    }
    if (bitboard >> 7 & NOT_A_FILE) {
        attacks |= (bitboard >> 7);   
    }
    if (bitboard >> 1 & NOT_H_FILE) {
        attacks |= (bitboard >> 1);   
    }
    if (bitboard << 8) {
        attacks |= (bitboard << 8);   
    }
    if (bitboard << 9 & NOT_A_FILE) {
        attacks |= (bitboard << 9);   
    }
    if (bitboard << 7 & NOT_H_FILE) {
        attacks |= (bitboard << 7);   
    }
    if (bitboard << 1 & NOT_A_FILE) {
        attacks |= (bitboard << 1);   
    }
    return attacks;
}

// Relevant occupancy bit for bishop (last squares not included)
Bitboard mask_bishop_attacks(int square) {
    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    // relevent bishop occupancy bits
    for (rank = target_rank + 1, file = target_file + 1; rank <= 6 && file <= 6; rank++, file++) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
    }

    for (rank = target_rank - 1, file = target_file + 1; rank >= 1 && file <= 6; rank--, file++) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
    }

    for (rank = target_rank + 1, file = target_file - 1; rank <= 6 && file >= 1; rank++, file--) {
        attacks |= (1ULL << SQUARE_INDEX(rank, file));
    }

     for (rank = target_rank - 1, file = target_file - 1; rank >= 1 && file >= 1; rank--, file--) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
    }
    return attacks;
}

Bitboard generate_bishop_attacks(int square, Bitboard block) {
    Bitboard attacks = 0ULL;

    int rank, file;
    int target_rank = square / 8;
    int target_file = square % 8;

    // generate attacks
    for (rank = target_rank + 1, file = target_file + 1; rank <= 7 && file <= 7; rank++, file++) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
        // If the piece runs into another piece, the squares after that are unreachable, so the piece cannot move any further. Break the loop.
        // This function assumes the "blocking" piece can be captured. This will be decided by the move generator.
        if ((1ULL << (SQUARE_INDEX(rank, file))) & block) {
            break;
        }
    }

    for (rank = target_rank - 1, file = target_file + 1; rank >= 0 && file <= 7; rank--, file++) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
        if ((1ULL << (SQUARE_INDEX(rank, file))) & block) {
            break;
        }
    }

    for (rank = target_rank + 1, file = target_file - 1; rank <= 7 && file >= 0; rank++, file--) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
        if ((1ULL << (SQUARE_INDEX(rank, file))) & block) {
            break;
        }
    }

     for (rank = target_rank - 1, file = target_file - 1; rank >= 0 && file >= 0; rank--, file--) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, file)));
        if ((1ULL << (SQUARE_INDEX(rank, file))) & block) {
            break;
        }
    }
    return attacks;
}

// Relevant occupancy bit for rook (last squares not included)
Bitboard mask_rook_attacks(int square) {
    Bitboard attacks = 0ULL;
    
    int rank, file;
    
    int target_rank = square / 8;
    int target_file = square % 8;
    
    // relevent rook occupancy bits
    for (rank = target_rank + 1; rank <= 6; rank++) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, target_file)));
    }
    for (rank = target_rank - 1; rank >= 1; rank--) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, target_file)));
    }
    for (file = target_file + 1; file <= 6; file++) {
        attacks |= (1ULL << (SQUARE_INDEX(target_rank, file)));
    }
    for (file = target_file - 1; file >= 1; file--) {
        attacks |= (1ULL << (SQUARE_INDEX(target_rank, file)));
    }    
    return attacks;
}

Bitboard generate_rook_attacks(int square, Bitboard block) {
    Bitboard attacks = 0ULL;
    
    int rank, file;
    
    int target_rank = square / 8;
    int target_file = square % 8;
    
    //generate attacks
    for (rank = target_rank + 1; rank <= 7; rank++) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, target_file)));
        // If the piece runs into another piece, the squares after that are unreachable, so the piece cannot move any further. Break the loop.
        // This function assumes the "blocking" piece can be captured. This will be decided by the move generator.
        if ((1ULL << (SQUARE_INDEX(rank, target_file))) & block) {
            break;
        }
    }
    for (rank = target_rank - 1; rank >= 0; rank--) {
        attacks |= (1ULL << (SQUARE_INDEX(rank, target_file)));
         if ((1ULL << (SQUARE_INDEX(rank, target_file))) & block) {
            break;
        }
    }
    for (file = target_file + 1; file <= 7; file++) {
        attacks |= (1ULL << (SQUARE_INDEX(target_rank, file)));
         if ((1ULL << (SQUARE_INDEX(target_rank, file))) & block) {
            break;
        }
    }
    for (file = target_file - 1; file >= 0; file--) {
        attacks |= (1ULL << (SQUARE_INDEX(target_rank, file)));
         if ((1ULL << (SQUARE_INDEX(target_rank, file))) & block) {
            break;
        }
    }    
    return attacks;
}

void init_siders(int is_bishop) {
    for (int s = 0; s < 64; s++) {
        bishop_masks[s] = mask_bishop_attacks(s);
        rook_masks[s] = mask_rook_attacks(s);

        Bitboard attack_mask = is_bishop ? bishop_masks[s] : rook_masks[s];

        // Relevant occupany bit
        int relevant_bit_count = count_bits(attack_mask);

        // Slider occupancy
         int occupancy_indicies = (1 << relevant_bit_count);

         for (int i = 0; i < occupancy_indicies; i++) {
            Bitboard occupancy = set_occupancy(i, relevant_bit_count, attack_mask);
            if (is_bishop) {
                int magic_index = (occupancy * bishop_magics[s]) >> (64 - bishop_relevant_bits[s]);
                bishop_attacks[s][magic_index] = generate_bishop_attacks(s, occupancy);
            } else {
                int magic_index = (occupancy * rook_magics[s]) >> (64 - rook_relevant_bits[s]);
                rook_attacks[s][magic_index] = generate_rook_attacks(s, occupancy);
            }
         }
    }
}

Bitboard get_bishop_attacks(int square, Bitboard occupancy) {
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magics[square];
    occupancy >>= 64 - bishop_relevant_bits[square];
    return bishop_attacks[square][occupancy];
}

Bitboard get_rook_attacks(int square, Bitboard occupancy) {
    occupancy &= rook_masks[square];
    occupancy *= rook_magics[square];
    occupancy >>= 64 - rook_relevant_bits[square];
    return rook_attacks[square][occupancy];
}

Bitboard get_queen_attacks(int square, Bitboard occupancy) {
    return (get_bishop_attacks(square, occupancy) | get_rook_attacks(square, occupancy));
}


/* 
 index is the range of occupancy in bits (eg 1 = a8, 2 = a7, 3 = a8 + a7, etc)
 bits_mask is the count of bits in the attack mask provided
 returns a bitboard with the occupancy along with given the range of bits in the mask (piece available moves) 
 The occupancies are calculated based on the mask (theoretically available moves)

 Essentially, this method returns all possible combinations of occupancies in the path of the piece's attack mask (pieces in the way of the masked piece's moves). 
*/
Bitboard set_occupancy(int index, int bits_mask, Bitboard attack_mask) {
    Bitboard occupancy = 0ULL;

    // Loop over the bits within the mask
    for (int i = 0; i < bits_mask; i++) {
        // Get least significant 1st bit index of attack mask and pop it
        int square = get_least_sig_bit_index(attack_mask);
        pop_bit(attack_mask, square);

        // Ensure occupancy is on board, then populate map
        if (index & (1 << i)) {
            occupancy |= (1ULL << square);
        }
    }

    return occupancy;
}

// Generate attack tables
void init_tables() {
    for (int square = 0; square < 64; square++) {
        // Pawn attacks
        pawn_attacks[WHITE][square] = mask_pawn_attacks(WHITE, square);
        pawn_attacks[BLACK][square] = mask_pawn_attacks(BLACK, square);

        // Kinght attacks
        knight_attacks[square] = mask_knight_attacks(square);

        // King attacks
        king_attacks[square] = mask_king_attacks(square);
    }

    init_siders(BISHOP);
    init_siders(ROOK);
}



void print_bitboard(Bitboard bitboard) {

    printf("\n");

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = SQUARE_INDEX(rank, file);
            if (!file) {
                printf(" %d ", 8 - rank);
            }
            printf(" %d ", get_bit(bitboard, square) ? 1 : 0);
        }
        printf("\n");
    }

    // print files
    printf("\n    a  b  c  d  e  f  g  h  \n\n");

    // print bitboard as unsigned decimal
    printf("Bitboard: %llud\n\n", bitboard);
}
