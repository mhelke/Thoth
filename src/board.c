#include <stdio.h>
#include <string.h>

#include "bitboard.h"

void reset_board(Board *board) {
    memset(board->bitboards, 0ULL, sizeof(board->bitboards));
    memset(board->occupancies, 0ULL, sizeof(board->occupancies));
    board->side = 0;
    board->enpassant = na;
    board->castle = 0;
}

void load_fen(char* fen, Board *board) {
    reset_board(board);

    int i = 0;
    int n = strlen(fen);
    int rank = 0;
    int file = 0;
    for (; i < n; i++) {
        int done = 0;
        switch (fen[i]) {
            case 'P': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'N': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'B': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'R': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'Q': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'K': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'p': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'n': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'b': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'r': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'q': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case 'k': SET_BIT(board->bitboards[char_pieces[fen[i]]], (SQUARE_INDEX(rank, file++))); break;
            case '/': file = 0; rank++; break;
            case '1': file += 1; break;
            case '2': file += 2; break;
            case '3': file += 3; break;
            case '4': file += 4; break;
            case '5': file += 5; break;
            case '6': file += 6; break;
            case '7': file += 7; break;
            case '8': file += 8; break;
            case ' ': rank += 1; done = 1; break;
            default: return;
        }
        if (done) {
            if (rank != 8 || file != 8) {
                return;
            }
            break;
        }
    }
    i++;
    switch (fen[i++]) {
        case 'w':
            board->side = WHITE;
            break;
        case 'b':
            board->side = BLACK;
            break;
        default: return;
    }
    i++;
    board->castle = 0;
    for (; i < n; i++) {
        int done = 0;
        switch (fen[i]) {
            case 'K': board->castle |= WK; break;
            case 'Q': board->castle |= WQ; break;
            case 'k': board->castle |= BK; break;
            case 'q': board->castle |= BQ; break;
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
        board->enpassant = na;
        i++;
    } else if (fen[i] >= 'a' && fen[i] <= 'h') {
        int ep_file = fen[i] - 'a';
        i++;
        if (fen[i] >= '1' && fen[i] <= '8') {
            int ep_rank = 8 - (fen[i] - '0');
            board->enpassant = SQUARE_INDEX(ep_rank, ep_file);;
            i++;
        }
    } else {
        board->enpassant = na;
    }

    for (int piece = P; piece <= K; piece++) {
        board->occupancies[WHITE] |= board->bitboards[piece];
    }

    for (int piece = p; piece <= k; piece++) {
        board->occupancies[BLACK] |= board->bitboards[piece];
    }
    board->occupancies[BOTH] |= board->occupancies[WHITE];
    board->occupancies[BOTH] |= board->occupancies[BLACK];
    i++;
}

void print_board(Board *board) {
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
                if (GET_BIT(board->bitboards[i], square)) {
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
    
    printf("Side to move:\t %s\n", !board->side ? "White" : "Black");
    
    printf("Enpassant:\t %s\n", (board->enpassant != na) ? square[board->enpassant] : "N/A");
    
    printf("Castle:\t\t %c%c%c%c\n\n", (board->castle & WK) ? 'K' : '-',
                                           (board->castle & WQ) ? 'Q' : '-',
                                           (board->castle & BK) ? 'k' : '-',
                                           (board->castle & BQ) ? 'q' : '-');    
}
