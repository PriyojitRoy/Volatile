#ifndef VENGINE_SEARCH_NNUE_MCTS_H
#define VENGINE_SEARCH_NNUE_MCTS_H

#include "common.h"
#include "core/Board.hpp"
#include "core/Move.hpp"
#include "search/nnue/Node.hpp"
#include <chrono>

namespace VEngine {

    class MCTS {
    public:
        MCTS();
        ~MCTS();

        Move getBestMove(Board& board, int optimumTime, int maximumTime);
        std::atomic<bool> stopSearch{false};
        std::atomic<bool> isPondering{false};
        int lastEvalScore = 0;
        uint64_t maxNodesLimit = 0;
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
    private:
        int nodesSearched;
        int allocatedTime;
    };

}

#endif // VENGINE_SEARCH_NNUE_MCTS_H
