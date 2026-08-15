#include "eval/nnue/Network.hpp"

namespace VEngine {

    int Network::evaluate(const Board& board) {
        // TODO: Implement NNUE evaluation
        (void)board;
        return 0;
    }

    int Network::lazyEvaluate(const Board& board) {
        // TODO: Implement NNUE lazy evaluation
        (void)board;
        return 0;
    }

    bool Network::load(const std::string& path) {
        std::cout << "Loading NNUE network from: " << path << " (Placeholder)" << std::endl;
        return true;
    }

    const std::vector<float>& Network::getBiases() {
        static std::vector<float> biases;
        return biases;
    }

    void Network::computeLayer(float* input, float* weights, float* biases, float* output, int inSize, int outSize, bool activate) {
        (void)input; (void)weights; (void)biases; (void)output; (void)inSize; (void)outSize; (void)activate;
    }
}
