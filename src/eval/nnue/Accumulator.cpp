#include "eval/nnue/Accumulator.hpp"
#include <cstring>

namespace VEngine {
    Accumulator::Accumulator() {
        white.fill(0);
        black.fill(0);
    }
    
    void Accumulator::init(const std::vector<int16_t>& biases) {
         if (biases.size() < 256) return;
         std::memcpy(white.data(), biases.data(), 256 * sizeof(int16_t));
         std::memcpy(black.data(), biases.data(), 256 * sizeof(int16_t));
    }
    
    void Accumulator::updateSpecific(int index, int perspective, const int16_t* weights) {
        // TODO: Replace this scalar loop with _mm256_add_epi16 (AVX2) for extreme speed
        const int16_t* featureWeights = &weights[index * 256];    
        int16_t* target = (perspective == 0) ? white.data() : black.data();
        for (int i = 0; i < 256; i++) {
            target[i] += featureWeights[i];
        }
    }
}