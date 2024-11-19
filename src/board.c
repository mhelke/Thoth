#include <stdio.h>
#include <string.h>

#include "bitboard.h"


void reset_board() {
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    side = 0;
    enpassant = na;
    castle = 0;

}

void load_fen(char* fen) {
    reset_board();

    int i = 0;
    int n = strlen(fen);
    int rank = 7;
    int file = 0;
    for (; i < n; i++) {
        int done = 0;
        switch (fen[i]) {
            case 'P': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'N': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'B': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'R': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'Q': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'K': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'p': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'n': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'b': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'r': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'q': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'k': set_bit(bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case '/': file = 0; rank--; break;
            case '1': file += 1; break;
            case '2': file += 2; break;
            case '3': file += 3; break;
            case '4': file += 4; break;
            case '5': file += 5; break;
            case '6': file += 6; break;
            case '7': file += 7; break;
            case '8': file += 8; break;
            case ' ': done = 1; break;
            default: return;
        }
        if (done) {
            if (rank != 0 || file != 8) {
                return;
            }
            break;
        }
    }
    i++;
    switch (fen[i++]) {
        case 'w':
            side = WHITE;
            break;
        case 'b':
            side = BLACK;
            break;
        default: return;
    }
    i++;
    castle = 0;
    for (; i < n; i++) {
        int done = 0;
        switch (fen[i]) {
            case 'K': castle |= WK; break;
            case 'Q': castle |= WQ; break;
            case 'k': castle |= BK; break;
            case 'q': castle |= BQ; break;
            case '-': done = 1; break;
            case ' ': done = 1; break;
            default: return;
        }
        if (done) {
            break;
        }
    }
    i++;
    if (fen[i] == '-') {
        enpassant = na;
        i++;
    } else if (fen[i] >= 'a' && fen[i] <= 'h') {
        int ep_file = fen[i] - 'a';
        i++;
        if (fen[i] >= '1' && fen[i] <= '8') {
            int ep_rank = 8 - (fen[i] - '0');
            enpassant = SQUARE_INDEX(ep_rank, ep_file);;
            i++;
        }
    } else {
        enpassant = na;
    }
    i++;
}

void print_board() {
    printf("\n");

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = SQUARE_INDEX(rank, file);;
            
            if (!file) {
                printf("  %d ", 8 - rank);
            }
            
            int piece = -1;
            
            // Loop over all bitboards for each piece type and get the bit
            for (int i = P; i <= k; i++) {
                if (get_bit(bitboards[i], square)) {
                    piece = i;
                }
            }
            // Print the piece notation for the given bitboard 
            printf("  %c", (piece == -1) ? '.' : ascii_pieces[piece]);
        }
        printf("\n");
    }
    
    // print board files
    printf("\n      a  b  c  d  e  f  g  h\n\n");
    
    printf("Side to move:\t %s\n", !side ? "White" : "Black");
    
    printf("Enpassant:\t %s\n", (enpassant != na) ? square[enpassant] : "N/A");
    
    printf("Castle:\t\t %c%c%c%c\n\n", (castle & WK) ? 'K' : '-',
                                           (castle & WQ) ? 'Q' : '-',
                                           (castle & BK) ? 'k' : '-',
                                           (castle & BQ) ? 'q' : '-');    
}
