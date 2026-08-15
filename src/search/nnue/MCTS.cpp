#include "search/nnue/MCTS.hpp"
#include "core/MoveGen.hpp"

namespace VEngine {

    MCTS::MCTS() : nodesSearched(0), allocatedTime(0) {
    }

    MCTS::~MCTS() {
    }

    Move MCTS::getBestMove(Board& board, int optimumTime, int maximumTime) {
        // TODO: Implement actual Monte Carlo Tree Search for NNUE here
        (void)optimumTime;
        (void)maximumTime;
        
        MoveList initialMoves;
        MoveGen::generateAllMoves(board, initialMoves);

        // Fallback to the first legal move for now
        for (int i = 0; i < initialMoves.size(); i++) {
            Move m = initialMoves.moves[i];
            if (board.makeMove(m)) {
                board.unmakeMove(m);
                return m;
            }
        }
        
        return Move(); // No legal moves
    }
}
