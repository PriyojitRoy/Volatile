#include "eval/nnue/Network.hpp"
#include <iostream>

namespace VEngine {

    int Network::evaluate(const Board& board) {
        // TODO: Implement the dense layer forward pass inside Network::evaluate() 
        // to collapse the 256-value accumulator into a single centipawn score.
        (void)board;
        return 0;
    }

    int Network::lazyEvaluate(const Board& board) {
        return evaluate(board);
    }

    bool Network::load(const std::string& path) {
        // TODO: Write the Network::load() function to actually read a .nnue binary file.
        std::cout << "Loading NNUE network from: " << path << " (Placeholder)" << std::endl;
        return true;
    }

    const std::vector<int16_t>& Network::getBiases() {
        static std::vector<int16_t> biases(256, 0);
        return biases;
    }

    void Network::computeLayer(int16_t* input, int16_t* weights, int32_t* biases, int32_t* output, int inSize, int outSize, bool activate) {
        // TODO: Update layer computation to use SIMD instructions
        (void)input; (void)weights; (void)biases; (void)output; (void)inSize; (void)outSize; (void)activate;
    }
}
