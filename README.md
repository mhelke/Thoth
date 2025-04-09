# Thoth
Named after the Egyptian god of the moon, knowledge, and wisdom, Thoth is an open-source chess engine written in C.

Thoth does not have it's own GUI, but it supports the [Universal Chess Interface (UCI) protocol](https://www.chessprogramming.org/UCI). It can be run with any UCI-compatible chess GUI such as [Arena](http://www.playwitharena.de/) or [Cute Chess](https://github.com/cutechess/cutechess).

## Performance
  * Elo: 2057-2169
    
_Elo is calculated by playing games vs engines from [CCRL](http://computerchess.org.uk/ccrl/4040/index.html). At minimum 100 games are played for each test using a 40/15 repeating time control. A rating range based on the results is calculated from Cute Chess using the known rating of the other engine. Thoth's elo is given as a range with a margin of error because its testing is not as thorough as CCRL's._

## Features
  * Bitboards and magic bitboards to represent the chessboard and allow for super-fast and efficient move generation
  * Alpha-Beta algorithm with quiescence search
  * Iterative deepening with aspiration window
  * Null move pruning
  * PV search with late move reduction
  * UCI-compatible PV lines, including mate scores
  * Move ordering with Killer and History Heuristics
  * Delta pruning in quiescence search
  * Static Exchange Evaluation (SEE) using the Swap Algorithm
  * Zobrist hashing and transposition tables with dynamic memory allocation
  * 3-fold repetition and 50-move rule draw detection
  * Tapered evaluation for game phase transitions
  * Futility pruning
  * Razoring
  * Single reply and check extensions
  * Compatible for playing with a time control, fixed depth, or move time

## Evaluation Metrics
 * Material scores
 * Piece-square tables by game phase
 * Mobility
 * Pawn structure
 * Rook placement
 * King safety
 * Minor piece imbalances
  
## Installation 

#### Option 1 - Download the release
1. Download the latest release and import the executable into a UCI-compatible program such as [Arena](http://www.playwitharena.de/)
2. Watch the engine play!

#### Option 2 - Compile the engine
1. From the root directory, run `make` to generate the executable
2. Import the executable into a UCI-compatible program such as [Arena](http://www.playwitharena.de/)
3. Watch the engine play!

## Command Line Usage
       thoth.exe <MODE> <POSITION> <DEPTH>
   MODE
   * empty - The engine runs with the UCI protocol and can be loaded into a GUI. All other run arguments are ignored. Interaction with the engine can be done using UCI commands.
   * `debug` - The engine will search the specified `position` up to the specified `depth` and output
       1. A diagram of the board
       2. The PV line for each depth searched
       3. Static evaluation metrics for the position
   * `test` - Executes the perft tests and the tests in `tests.c`
     
   POSITION
   * The position to evaluate, in FEN format. It must be a valid FEN string.
   * If no FEN is given, a default position will be used.
   * Only used when the `mode` is `debug`.
   
   DEPTH
   * How many moves to search. If none is given, a default for 10 is used.
   * Only used when the `mode` is `debug`.

## Author

Matthew Helke

* Contact: [matthewhelke52@gmail.com](mailto:matthewhelke52@gmail.com)
* Github: [mhelke](https://github.com/mhelke)
* LinkedIn: [matthew-helke](https://www.linkedin.com/in/matthew-helke)
* Chess.com: [MasterMatthew52](https://www.chess.com/member/mastermatthew52)

## Contributing

Feedback on implementation and issues are welcome!
Please add your request to the [issues page](https://github.com/mhelke/Thoth/issues)

## License

This project is [MIT licensed](https://github.com/mhelke/Thoth/blob/master/LICENSE)
