# Thoth
Thoth is a UCI-compatible chess engine written in C.

## Performance
  * Elo:

## Features
  * Bitboards and magic bitboards to represent the chessboard and allow for super-fast and efficient move generation
  * Alpha-Beta algorithm with quiescence search
  * Iterative deepening with aspiration window
  * Null move pruning
  * Late move reduction and PV search
  * UCI-compatible PV lines, including mate scores
  * Move ordering with Killer and History Heuristics
  * Delta pruning in quiescence search
  * Static Exchange Evaluation (SEE) using the Swap Algorithm
  * Zobrist hashing and transposition tables with dynamic memory allocation
  * 3-fold repetition and 50-move rule draw detection
  * Tapered evaluation for game phase transitions
  * Futility pruning
  * Compatible for playing with a time control, fixed depth, or move time

## Evaluation Metrics
 * Material scores
 * Piece-square tables
 * Mobility
 * Pawn structure
 * Rook placement
 * King safety
  
## Running the Engine

#### Option 1 - Download the release
1. Download the latest release and import into a program such as [Arena](http://www.playwitharena.de/).
2. Watch the engine play!

#### Option 2 - Compile the engine
1. From the root directory, run `make` to generate the executable.
2. Import the executable into a program such as [Arena](http://www.playwitharena.de/).
3. Watch the engine play!

## Fixed Position from the Command Line
1. From the root directory, run `make` to generate the executable.
2. Run the executable from the command line, specifying debug mode, an FEN, and search depth

       thoth.exe debug <FEN> <DEPTH>
   If an FEN is not provided, a default position with a depth of 10 is used.
3. The PV lines and a static evaluation will output to the terminal.

## Author

Matthew Helke

* Contact: [matthewhelke52@gmail.com](mailto:matthewhelke52@gmail.com)
* Github: [mhelke](https://github.com/mhelke)
* LinkedIn: [matthew-helke](https://www.linkedin.com/in/matthew-helke)
* Chess.com: [MasterMatthew52](https://www.chess.com/member/mastermatthew52)
* Lichess.org: [MasterMatthew52](https://lichess.org/@/MasterMatthew52)

## Contributing

Feedback on implementation or issues is welcome!
Please add your request to the [issues page](https://github.com/mhelke/Thoth/issues)

## License

Copyright (c) 2025 Matthew Helke
