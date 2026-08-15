#ifndef CHESS_EVAL_NNUE_ACCUMULATOR_H
#define CHESS_EVAL_NNUE_ACCUMULATOR_H
#include <array>
#include <vector>
#include <cstdint>

namespace VEngine {

    constexpr int NetworkHiddenSize = 256; 

    struct Accumulator {
        std::array<float, NetworkHiddenSize> white;
        std::array<float, NetworkHiddenSize> black;

        Accumulator();
        void init(const std::vector<float>& biases);
        void updateSpecific(int index, int perspective, const float* weights);
    };
}

#endif // CHESS_EVAL_NNUE_ACCUMULATOR_H