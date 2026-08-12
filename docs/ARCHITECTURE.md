# Volatile Architecture

This document provides a deep-dive into the architectural design of the Volatile chess engine. It is specifically tailored for AI agents and human contributors who need to understand the codebase efficiently to make meaningful contributions, refactors, and optimizations.

## 1. High-Level Design Principles

- **Separation of Concerns:** The engine strongly decouples the core chess logic (board representation, move generation) from the evaluation and search backends.
- **Backend Dichotomy (HCE vs. NNUE):** Volatile is fundamentally split between two evaluation and search paradigms:
  - **HCE (Hand-Crafted Evaluation):** The traditional alpha-beta/negamax search using manually tuned evaluation functions (material, PSTs, mobility, etc.).
  - **NNUE (Efficiently Updatable Neural Networks):** An advanced neural network evaluator primarily paired with a Monte-Carlo Tree Search (MCTS) backend.
- **Build-Time Polymorphism:** To maximize performance, Volatile avoids runtime virtual dispatch where possible. The switch between HCE and NNUE is controlled via CMake (`USE_NNUE=ON` vs. `USE_NNUE=OFF`). The build system conditionally compiles the correct `src/` subdirectories.

## 2. Directory and Module Breakdown

The codebase is split into `include/chess/` (declarations) and `src/` (definitions), grouped by logical modules.

### 2.1 Core (`core/`)

This module defines the rules and state of chess. 

- **`Constants.hpp`**: Centralized definitions for enums (`Color`, `PieceType`, `Square`, `Direction`) and global `constexpr` values. **Rule:** Never use `static` for constants that need external linkage.
- **`Bitboard.hpp / .cpp`**: 64-bit integer bitboard operations, masking, shifting, and population counts. Essential for fast move generation.
- **`Move.hpp / .cpp`**: Move encoding/decoding (typically using bitfields for from-square, to-square, promotion piece, and special flags like castling/en-passant).
- **`MoveGen.hpp / .cpp`**: Generates pseudo-legal and legal moves. Often leverages magic bitboards for sliding pieces.
- **`Board.hpp`**: The central state of the game. Contains bitboards for pieces, castling rights, en-passant square, and half-move clocks.
  - *Implementation Split:* Because HCE requires incremental updates to evaluation scores (e.g., updating a running PST score upon making a move), the `Board` implementation is physically split between `src/core/hce/` and `src/core/nnue/`. The NNUE variant avoids tracking HCE state, thereby saving CPU cycles during network rollouts.

### 2.2 Evaluation (`eval/`)

Handles static position evaluation.

- **`eval/hce/`**: Contains the Hand-Crafted Evaluator.
  - Features: Piece-Square Tables (PSTs), phased piece values (midgame vs. endgame), material counting, and mobility bonuses.
  - Updates: Operates incrementally. When a move is made, the evaluation is updated based on the piece moved and captured, rather than recalculated from scratch.
- **`eval/nnue/`**: Contains the NNUE integration.
  - **`Accumulator.hpp`**: Maintains the half-kp (king-piece) features. It incrementally updates the active features when pieces move.
  - **`Network.hpp`**: The neural network architecture and inference logic (often utilizing AVX2/AVX-512 SIMD instructions).

### 2.3 Search (`search/`)

The search module handles exploring the game tree to find the best move.

- **`search/hce/`**: Traditional Alpha-Beta search.
  - **`Search.hpp / .cpp`**: The main Principal Variation Search (PVS) and Negamax framework, using Iterative Deepening and Aspiration Windows.
  - **`Quiescence.cpp`**: Quiescence Search (QS) resolves tactical volatility at the leaf nodes (captures and promotions) to avoid the horizon effect.
  - **`MoveOrdering.cpp`**: Crucial for alpha-beta efficiency. Orders moves to maximize pruning. Implements Hash moves, SEE (Static Exchange Evaluation), Killer Moves, and History Heuristics.
  - **`TT.hpp / .cpp`**: Transposition Table with lockless hashing for caching exact scores and alpha/beta bounds.
  - **Pruning Techniques:** Null Move Pruning (NMP), Late Move Reductions (LMR), Reverse Futility Pruning (RFP), and ProbCut are heavily integrated.
- **`search/nnue/`**: 
  - Primarily focuses on Monte-Carlo Tree Search (MCTS) implementations to leverage the NNUE evaluations effectively, balancing exploration and exploitation (UCT).

### 2.4 UCI (`uci/`)

Universal Chess Interface implementation.
- Parses GUI commands (`position`, `go`, `setoption`, `isready`).
- Manages engine threads and standard input/output.
- Translates engine PVs (Principal Variations) and scores into UCI-compliant string responses.

### 2.5 External Integrations

- **`fathom/`**: A C library integration used for probing Syzygy endgame tablebases. Provides perfect play in endgame scenarios (<= 6 pieces).

## 3. Build System and Testing

- **CMake (`CMakeLists.txt`)**: The engine relies entirely on CMake. Modules are built into an `EngineLib` target. 
- **Testing (`tests/`)**: Uses CTest. Includes fundamental verifications like `TestBoard.cpp` and `TestMoveGen.cpp` (Perft testing).

## 4. Agent Guidelines for Efficient Modification

When you (the AI agent) modify this codebase, keep these architectural constraints in mind:

1. **Check the Build Target:** If you modify or create a file in `hce/`, ensure you are testing the `USE_NNUE=OFF` build. Conversely for `nnue/` and `USE_NNUE=ON`.
2. **File Size Limits (<700 lines):** Maintain granular separation. If `Search.cpp` grows beyond 700 lines, abstract components out (e.g., move reduction heuristics, time management).
3. **No Header Leaks:** Keep `#include` directives minimal in headers to reduce compilation times. Favor forward declarations (`class Board;`) where possible.
4. **Memory:** Avoid dynamic allocation (`malloc`/`new`) inside the search tree. Use stack allocation, pre-allocated vectors, or object pools.
5. **Single Commit Target:** Accumulate changes logically for the current task so they can be squashed cleanly prior to merging.
