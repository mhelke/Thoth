/*
    Thoth Engine
    © Matthew Helke 2024
*/

#include "bitboard.h"
#include "uci.h"

int main() {
    // Initialize move tables
    init_tables();
    uci_main();
}