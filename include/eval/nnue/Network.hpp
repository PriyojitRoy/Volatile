#ifndef CHESS_EVAL_NNUE_NETWORK_H
#define CHESS_EVAL_NNUE_NETWORK_H

#include <string>
#include <vector>
#include "core/Board.hpp"
#include "eval/nnue/Accumulator.hpp"

namespace VEngine {
    
    class Network {
    public:
        static int evaluate(const Board& board);
        static int lazyEvaluate(const Board& board);
        static bool load(const std::string& path);
        static const std::vector<float>& getBiases();

        // NNUE specific
        static void computeLayer(float* input, float* weights, float* biases, float* output, int inSize, int outSize, bool activate);
    }; 
}

#endif // CHESS_EVAL_NNUE_NETWORK_H
