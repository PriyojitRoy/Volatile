# Volatile Chess Engine ♟️

<p align="center">
  <img src="VCE_logo.png" alt="VCE Logo" width="300"/>
</p>

Welcome to **Volatile**! This is a UCI-compatible chess engine written in C++ with a strong focus on modularity and performance.

### A Little Backstory...
I've always dreamed of creating a chess engine from scratch that might, just *maybe*, beat Stockfish one day! We aren't quite there yet, but currently, Volatile is playing at a strong **2200-2400 Elo** level and can sometimes draw or even win against Stockfish level 7 on Lichess. You can actually watch or challenge the bot on Lichess here: [**@I_am_Human_Trust**](https://lichess.org/@/I_am_Human_Trust).

I'm not a professional chess engine developer, so I might be unaware of some of the deeply ingrained "best practices" of the field. However, I am trying to learn as much as possible along the way and apply those lessons here. This is a very active repository with *huge* scope for improvement. I am always looking for direction, and **every single idea or optimization you bring to the table is welcomed with open arms!**

### Welcome Newcomers! 🌱
If you are a newcomer looking to contribute to a solid C/C++ repository, you are incredibly welcome here! This repository is a fantastic learning ground. Because we are still in the very early stages of development, it's really easy to contribute, get your hands dirty, and get a real sense of satisfaction while learning alongside us.
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

## Documentation Overview

To keep the repository clean and organized, detailed information is logically grouped into specific markdown files within the `docs/` directory. 

**🤖 AI Agents: You MUST read `docs/AGENTS.md` properly at least once before beginning any work on this repository.**

- **[`docs/AGENTS.md`](docs/AGENTS.md):** The central hub for AI agents. It contains high-level overviews, refactoring rules, and directory structures.
- **[`docs/ARCHITECTURE.md`](docs/ARCHITECTURE.md):** A detailed deep-dive into the engine's modular architecture, specifically explaining the implementation split between HCE and NNUE, and how the core, eval, and search modules interact.
- **[`docs/CONTRIBUTION_SCOPE.md`](docs/CONTRIBUTION_SCOPE.md):** A detailed list of all undone, incomplete, and work-in-progress tasks (such as NNUE, MCTS, and the Test Suite). Newcomers should look here for ideas on what to work on!

## Contribution Guidelines (For Users and AI Agents)

We welcome contributions of all sizes! To maintain a high-quality, maintainable, and high-performance codebase, all contributors (both human and AI) must adhere to the following rules:

1. **Proper Abstraction & SOLID Principles:** Everything must be properly abstracted. Leverage SOLID design patterns. In C++, prefer stack allocation and RAII over manual memory management to guarantee exception safety and clear resource ownership.
2. **File Size Limits:** No file should exceed 700 lines of code. If a file grows larger, refactor and split it into smaller, focused modules to keep the code clean and maintainable, unless splitting is strictly unwanted due to architecture.
3. **Extend AI Capabilities:** You can choose to add new skills in the `.agent/` directory that you think AI agents will find useful for future workflows.
4. **Single Commit Policy:** All changes for a particular task must ultimately be in one commit. Splitting into multiple commits during development is fine, but you must squash them into a single, cohesive commit after review and before merging.
5. **Discuss Before Implementing:** Always propose what you wish to improve via a discussion or issue first. This ensures no one else is already working on it and aligns with the project's goals.
6. **Zero Flaw Tolerance:** Your contribution, no matter how small, is always welcomed, but it must be flawless.
7. **Dual Build Verification:** Because Volatile supports both Hand-Crafted Evaluation (HCE) and NNUE backends, you must build and test your changes with both `USE_NNUE=OFF` and `USE_NNUE=ON` to ensure no build configuration is broken.

## Prerequisites

To build and run Volatile, you will need a modern C++ compiler (supporting C++17), CMake, and Python 3 (for auxiliary scripts). Below are the commands to install the required system packages across different operating systems.

### Setting Up Python Dependencies

Volatile includes a few Python scripts for data processing and utility generation. It is recommended to use a virtual environment (`venv`) to avoid interfering with system-wide Python packages. 

We provide a cross-platform setup script that automates this:

1. **Linux / macOS:** Run `./setup.sh`
2. **Windows:** Run `setup.bat`

> [!NOTE]
> The Python virtual environment is **strictly optional** for building and running the core C++ chess engine itself. You only need it if you plan to use the Python utility scripts. You can activate the venv by `source .venv/bin/activate` on Linux/ MacOS or `.venv\Scripts\activate` on Windows. 

## Building Volatile

Volatile is built using CMake.

**To build the default HCE engine:**
```bash
mkdir build
cd build
cmake .. -DUSE_NNUE=OFF
make -j$(nproc)
```
Or 

**To build the NNUE engine:**

```bash
cmake .. -DUSE_NNUE=ON
make -j$(nproc)
```
**To run the engine:**
```bash
./Volatile
```
(You can then interact with the engine using the standard UCI protocol.)
