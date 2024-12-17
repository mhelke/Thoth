# Thoth
Thoth is a UCI-compatible chess engine written in C.

## Performance
  * Elo:

## Features
  * Bitboards and magic bitboards to represent the chessboard and allow for super fast and efficient move generation.
  * Alpha-Beta algorithm with quiescence search
  * Iterative deepening with aspiration window
  * Null move pruning
  * Late move reduction and PV search
  * UCI compatible PV lines 
  * Move ordering with Killer and History Heuristics
  * Zobrist hashing and transposition tables

## Evaluation Metrics
 * Material scores
 * Piece-square tables 
  
## Running the Engine

1. From the root directory, run `make` to generate the executable.
2. Import the executable into a program such as [Arena](http://www.playwitharena.de/).
3. Watch the engine play!

## Author

Matthew Helke

* Contact: [matthewhelke52@gmail.com](mailto:matthewhelke52@gmail.com)
* Github: [mhelke](https://github.com/mhelke)
* LinkedIn: [matthew-helke](https://www.linkedin.com/in/matthew-helke)

## Contributing

Feedback on implementation or issues is welcome!
Please add your request to the [issues page](https://github.com/mhelke/Thoth/issues)

## License

Copyright (c) 2024 Matthew Helke
