# Volatile - Agent Guidelines & Codebase Overview

Welcome! This file provides a low-token summary of the `Volatile` repository architecture. Read this file to understand the system quickly without needing to grep or list directories extensively.

## High-Level Architecture
This chess engine is written in modern C++ and supports two distinct evaluation backends: 
1. **HCE (Hand-Crafted Evaluation):** The default backend. Uses piecewise material values, PSTs (Piece-Square Tables), and classic alpha-beta/negamax search.
2. **NNUE (Efficiently Updatable Neural Networks):** An alternative backend selected via CMake flags.

The core architecture strictly separates generic chess concepts from backend-specific optimizations.

## Directory Structure
- **`include/chess/`**: Public headers.
  - `core/`: Core definitions (`Board.hpp`, `Move.hpp`, `Bitboard.hpp`, `Constants.hpp`, `MoveGen.hpp`).
  - `eval/`: Headers for evaluation (`hce/Evaluator.hpp`, `nnue/Accumulator.hpp`, `nnue/Network.hpp`).
  - `search/`: Headers for search (`hce/Search.hpp`, `hce/TT.hpp`, `nnue/MCTS.hpp`, etc.).
  - `uci/`: UCI protocol handler.
- **`src/`**: Implementation files mirroring the `include/` structure.
  - `core/`: Core mechanics (`Move.cpp`, `MoveGen.cpp`, `Bitboard.cpp`).
    - `hce/`: HCE-specific implementations of `Board.cpp`, `BoardMove.cpp`, `BoardUtils.cpp` (these update HCE eval states incrementally).
    - `nnue/`: NNUE-specific implementations of the `Board` (these do not update HCE scores, improving performance).
  - `eval/`: Evaluation logic divided into `hce/` and `nnue/`.
  - `search/`: Search logic.
    - `hce/`: Contains the traditional alpha-beta search. Split into `Search.cpp` (main loop), `Quiescence.cpp` (Q-Search), and `MoveOrdering.cpp` (SEE, killer moves, history). Also handles `Datagen`, `TT`, and Syzygy `Tablebase`.
    - `nnue/`: Contains Monte-Carlo Tree Search (`MCTS`) algorithms if enabled.
  - `fathom/`: Syzygy tablebase probing C library.
  - `uci/`: UCI implementation.
- **`tests/`**: CTest suite (e.g., `TestBoard.cpp`, `TestMoveGen.cpp`).

## Build Instructions
The project uses `CMake`. The target `EngineLib` statically links the modules, which `Volatile` and tests link against.

**Build HCE (Default):**
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

**Build NNUE:**
```bash
mkdir -p build && cd build
cmake -DCMAKE_BUILD_TYPE=Release -DUSE_NNUE=ON ..
make -j$(nproc)
```

## Refactoring Notes for Agents
- **`Board` Implementation**: The `Board` class is declared once in `include/chess/core/Board.hpp`, but its implementation is physically split between `src/core/hce/` and `src/core/nnue/`. CMake links the correct folder based on the `USE_NNUE` flag. Do not define implementations in `src/core/` directly if they are backend-dependent.
- **`Search` Implementation**: The HCE search logic is modular. `Search::quiescence` lives in `Quiescence.cpp`, `Search::orderMoves` and `Search::see` live in `MoveOrdering.cpp`, and the rest is in `Search.cpp`. Shared constants are declared as `constexpr` in `Search.hpp`.
- **Enums & Constants**: Global constants, `enum Color`, `enum PieceType`, `enum Square`, and `enum Direction` are centralized in `include/chess/core/Constants.hpp`.

## Code Guidelines
- Prioritize updating the appropriate `CMakeLists.txt` file when adding new `.cpp` files.
- Never use `static` for constants that need external linkage; use `constexpr` in header files instead.
- Preserve existing formatting and avoid changing working logic inside backend-specific folders unless requested.
