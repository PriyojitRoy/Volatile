# Contribution Scope

Welcome to the **Contribution Scope** document for Volatile. This file outlines the major areas of the codebase that are currently incomplete, heavily work-in-progress (WIP), or open for new features. 

If you are a newcomer (human or AI agent) looking for an idea of what to work on, this is the place to start. 

## 1. NNUE Integration (Efficiently Updatable Neural Networks)
**Status:** Incomplete / In Progress

The codebase architecture has been split to support a dual-backend evaluation (`USE_NNUE=ON` vs `OFF`), but the actual neural network evaluation logic still needs significant work.
- **Network Inference:** Implementing fast, SIMD-optimized (AVX2/AVX-512) network inference for the accumulator.
- **Training Data Pipeline:** We need scripts/methods for generating high-quality training data from the HCE engine.
- **Evaluation Tuning:** Replacing or supplementing the Hand-Crafted Evaluation (HCE) cleanly with the network's output.

## 2. MCTS Search (Monte-Carlo Tree Search)
**Status:** Incomplete / Pending

While traditional Alpha-Beta/Negamax is fully implemented for the HCE backend, NNUE evaluations often pair better with Monte-Carlo Tree Search. 
- **Tree Policy:** Implementing a robust UCT (Upper Confidence Bound applied to Trees) formula.
- **Rollout/Playout:** Handling leaf node evaluations via the NNUE network efficiently without updating HCE state.
- **Concurrency:** MCTS scales beautifully with threads. A lockless, multi-threaded MCTS implementation is highly desired.

## 3. Comprehensive Test Suite
**Status:** Undone / Needs Expansion

Currently, the testing suite in the `tests/` directory is minimal. It includes basic board verification and perft (movegen) tests, but it lacks extensive coverage.
- **Search Tests:** We need tests to verify that the search algorithms find forced mates, avoid obvious blunders, and handle edge cases correctly.
- **Evaluation Tests:** Tests to ensure symmetric evaluations (white vs. black) and proper incremental PST updates.
- **Endgame Tests:** Syzygy tablebase probing tests to ensure 100% accuracy in known endgames.
- **Unit Tests:** Expanding granular unit tests for core utilities (`Bitboard`, `Move`, `Zobrist`).

## 4. Code Maintenance and Refactoring
**Status:** Ongoing

As per our Contribution Guidelines, we enforce strict file size limits (<700 lines) and SOLID principles. 
- **Large Files:** Any file approaching or exceeding the 700-line limit should be carefully refactored and split into logical sub-components.
- **Modern C++ Upgrades:** Continuing to modernize the codebase using the latest C++ standards (C++20/23) where it benefits performance or readability.

If you decide to tackle any of these areas, please remember to open a discussion or issue first to ensure alignment with the project goals!

## 5. Infrastructure Improvements
**Status:** Ongoing

While we have a distinct folder separation for NNUE and HCE to maintain clean boundaries, the overall build and project infrastructure can definitely be improved further. Any enhancements to our build system, CI/CD, or project layout are welcome.

## 6. Time Management
**Status:** Needs Immediate Attention

Currently, alongside search optimizations, time management is one of our biggest issues. The engine's time management is really poor. Identifying the root causes contributing to this poor time management and implementing solid, robust time-allocation logic is very much desirable.

## 7. Opening Books & Data
**Status:** Open for Contributions

Currently, our `data/books` directory doesn't have many `.bin` files. If you can contribute newer, high-quality `.bin` polyglot files, opening books, or self-play datasets, it would be greatly appreciated.

## 8. Documentation & Tooling
**Status:** Poor / Needs Work

The overall documentation across the repository is currently very poor. We are looking for infrastructure updates to add tools like **Doxygen**, **GraphViz** (for generating architecture diagrams), and code formatters (like `clang-format`) to help keep the codebase readable, standardized, and well-documented.
