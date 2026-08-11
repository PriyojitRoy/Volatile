# Volatile Chess Engine

Volatile is a UCI-compatible chess engine written in C++ with a strong focus on modularity and performance.

## Current Features (HCE - Hand-Crafted Evaluation)

Volatile currently uses a Hand-Crafted Evaluation (HCE) backend that supports a robust and classic alpha-beta search algorithm.

**Core & Evaluation:**
- Fast bitboard-based move generation.
- Incremental evaluation updates (Zobrist hashing, material, and PST).
- Phased piece values and Piece-Square Tables (PSTs).
- History-based evaluation corrections.

**Search Algorithms:**
- Principal Variation Search (PVS) / Negamax framework.
- Iterative Deepening with Aspiration Windows.
- Quiescence Search with Delta Pruning.
- Transposition Table (TT) with lockless hashing.

**Pruning & Reductions:**
- Null Move Pruning (NMP).
- Late Move Reductions (LMR).
- Reverse Futility Pruning (RFP) / Static Null Move Pruning.
- ProbCut and Singular Extensions.

**Move Ordering:**
- Hash moves from TT.
- Static Exchange Evaluation (SEE) for capture ordering and pruning.
- Killer Move Heuristic.
- History Heuristic and Counter-Move Heuristic.

**Other Features:**
- **Syzygy Tablebases:** Integrated via Fathom for perfect endgame play.
- **Opening Book:** Support for Polyglot `.bin` opening books.
- **Data Generation:** Built-in tools to generate self-play data (`datagen` command).
- **Tuning:** Built-in SPSA tuning support (`tune` command).

## Work In Progress (To Be Done)

- **NNUE (Efficiently Updatable Neural Networks):** Implementation of an NNUE evaluation backend to replace/supplement HCE. The core architecture is already split to support this, but network integration and specific optimizations are pending.
- **MCTS Search:** Monte-Carlo Tree Search integration for the NNUE backend.
- **Test Suite:** A comprehensive testing suite is currently in progress (basic board and movegen tests exist but need expansion).

## Building Volatile

Volatile is built using CMake.

**To build the default HCE engine:**
```bash
mkdir build
cd build
cmake .. -DUSE_NNUE=OFF
make -j$(nproc)
```

**To run the engine:**
```bash
./Volatile
```
(You can then interact with the engine using the standard UCI protocol.)
