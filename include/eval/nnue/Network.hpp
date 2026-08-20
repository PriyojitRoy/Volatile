#ifndef VENGINE_EVAL_NNUE_NETWORK_H
#define VENGINE_EVAL_NNUE_NETWORK_H

#include <string>
#include <vector>
#include "core/Board.hpp"
#include "eval/nnue/Accumulator.hpp"

namespace VEngine {
    
    class Network {
    public:
        // TODO: Implement the dense layer forward pass inside Network::evaluate() 
        // to collapse the 256-value accumulator into a single centipawn score.
        static int evaluate(const Board& board);
        static int lazyEvaluate(const Board& board);
        
        // TODO: Write the Network::load() function to actually read a .nnue binary file.
        static bool load(const std::string& path);
        static const std::vector<int16_t>& getBiases();

        // NNUE specific
        // TODO: Update layer computation to use int16_t/int8_t SIMD
        static void computeLayer(int16_t* input, int16_t* weights, int32_t* biases, int32_t* output, int inSize, int outSize, bool activate);
    }; 
}

#endif // VENGINE_EVAL_NNUE_NETWORK_H
