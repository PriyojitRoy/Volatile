# Volatile - Agent Guidelines & Codebase Overview

Welcome! This file is the central hub for AI agents and human contributors working on the `Volatile` repository. To keep information logically grouped and token-efficient, the documentation is split across several files. 

## Documentation Map
Before diving into code, make sure you are aware of the following documents:
- **[README.md](../README.md):** The main entry point covering engine features, build instructions, and contribution rules.
- **[ARCHITECTURE.md](ARCHITECTURE.md):** A highly detailed breakdown of the engine's design, specifically how the core, evaluation, and search modules interact across the HCE and NNUE backends.
- **[CONTRIBUTION_SCOPE.md](CONTRIBUTION_SCOPE.md):** A detailed tracker of undone work, incomplete features, and open tasks (like NNUE training, MCTS, and testing). Check this to know what needs to be worked on!

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

## Contribution Guidelines (For Users and AI Agents)

We welcome contributions of all sizes! To maintain a high-quality, maintainable, and high-performance codebase, all contributors (both human and AI) must adhere to the following rules:

1. **Proper Abstraction & SOLID Principles:** Everything must be properly abstracted. Leverage SOLID design patterns. In C++, prefer stack allocation and RAII over manual memory management to guarantee exception safety and clear resource ownership.
2. **File Size Limits:** No file should exceed 700 lines of code. If a file grows larger, refactor and split it into smaller, focused modules to keep the code clean and maintainable, unless splitting is strictly unwanted due to architecture.
3. **Extend AI Capabilities:** You can choose to add new skills in the `.agent/` directory that you think AI agents will find useful for future workflows.
4. **Single Commit Policy:** All changes for a particular task must ultimately be in one commit. Splitting into multiple commits during development is fine, but you must squash them into a single, cohesive commit after review and before merging.
5. **Discuss Before Implementing:** Always propose what you wish to improve via a discussion or issue first. This ensures no one else is already working on it and aligns with the project's goals.
6. **Zero Flaw Tolerance:** Your contribution, no matter how small, is always welcomed, but it must be flawless..
7. **Dual Build Verification:** Because Volatile supports both Hand-Crafted Evaluation (HCE) and NNUE backends, you must build and test your changes with both `USE_NNUE=OFF` and `USE_NNUE=ON` to ensure no build configuration is broken.
8. **Git Commit Message Style:** All commit messages must follow the project's strict format:
   - The first line must use the Conventional Commits format (`type: description`, e.g., `feat: add ...` or `refactor: restructure ...`).
   - Leave an empty line after the first line.
   - Provide a detailed bulleted list of changes using hyphens (`-`). Each bullet point should explain a specific change and its reasoning.
   - Leave an empty line after the list.
   - End with a `Signed-off-by: Your Name <email@example.com>` signature block.
   - (Optional) Leave an empty line and add any closing/referencing issue tags (e.g., `Closes #7` or `Ref #2`).
# Agent Instructions for SPRT Testing

When performing search or evaluation enhancements, you **must** use the SPRT framework to prove the Elo gain before committing changes.

## Testing Protocol
If you are tweaking the search algorithms, here is the exact step-by-step workflow you should follow to prove your changes work:

1. **The Baseline:** Compile the unmodified engine (exactly as it is on the `main` branch) inside a folder named `build_base/`.
   ```bash
   mkdir -p build_base && cd build_base
   cmake ..
   make -j
   ```
2. **The Modification:** Edit the code in `src/` to add your search optimizations.
3. **The Test Engine:** Compile your new changes in the standard `build/` folder.
   ```bash
   mkdir -p build && cd build
   cmake ..
   make -j
   ```
4. **Configuration (Optional):** Open `tests/RunSPRT.cpp` and update the `runnerPath` if `fastchess` or `cutechess-cli` are in custom locations, or toggle `useFastChess` as appropriate.
5. **The Test Build:** Create a dedicated folder for testing (which will automatically detect it should build the tests instead of the main engine):
   ```bash
   cd ..
   mkdir build_tests && cd build_tests
   cmake ..
   make -j
   ```
6. **The Match:** From inside the `build_tests/` folder, run the SPRT match:
   ```bash
   ./run_sprt
   ```

The C++ code will automatically pit `../build_base/Volatile` against `../build/Volatile` using the standard `uho` opening book.
Do **not** commit search enhancements without documenting a passed SPRT test.
