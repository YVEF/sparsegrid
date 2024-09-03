SparseGrid is yet another UCI compatible chess engine, mostly the for experimantal purposes.<br>
The project enable the following features:
- Quad-Bitboards
- Magic board
- MTD(f) search
- Polyglot protocol adapter (classical chess only)
- Support up to 32 CPU threads
- UCI and FEN handling
- Handcrafted trainer based on PyTorch:
    - Supervised-like uses pgn database
    - Classic self-play RL with MCTS
