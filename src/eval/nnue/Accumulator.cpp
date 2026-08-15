#include "eval/nnue/Accumulator.hpp"
#include <cstring>

namespace VEngine {
    Accumulator::Accumulator() {
        white.fill(0.0f);
        black.fill(0.0f);
    }
    void Accumulator::init(const std::vector<float>& biases) {
         if (biases.size() < 256) return;
         std::memcpy(white.data(), biases.data(), 256 * sizeof(float));
         std::memcpy(black.data(), biases.data(), 256 * sizeof(float));
    }
    void Accumulator::updateSpecific(int index, int perspective, const float* weights) {
        const float* featureWeights = &weights[index * 256];    
        float* target = (perspective == 0) ? white.data() : black.data();
        for (int i = 0; i < 256; i++) {
            target[i] += featureWeights[i];
        }
    }
}