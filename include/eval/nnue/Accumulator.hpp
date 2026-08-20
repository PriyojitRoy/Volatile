#ifndef VENGINE_EVAL_NNUE_ACCUMULATOR_H
#define VENGINE_EVAL_NNUE_ACCUMULATOR_H

#include <array>
#include <vector>
#include <cstdint>

namespace VEngine {

    constexpr int NetworkHiddenSize = 256; 

    // Aligned to 64 bytes for optimal AVX2 / AVX-512 SIMD processing
    struct alignas(64) Accumulator {
        std::array<int16_t, NetworkHiddenSize> white;
        std::array<int16_t, NetworkHiddenSize> black;

        Accumulator();

        // TODO: Update parameters once Network.hpp defines the loaded weight structures
        void init(const std::vector<int16_t>& biases);
        
        // TODO: Implement incremental feature updates using SIMD (AVX2)
        // e.g. void addFeature(int featureIdx);
        // e.g. void removeFeature(int featureIdx);
        void updateSpecific(int index, int perspective, const int16_t* weights);
    };
}

#endif // VENGINE_EVAL_NNUE_ACCUMULATOR_H