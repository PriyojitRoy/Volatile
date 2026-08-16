#ifndef VENGINE_EVAL_HCE_EVALUATOR_H
#define VENGINE_EVAL_HCE_EVALUATOR_H
#include <string>
#include <vector>
#include "core/Board.hpp"

namespace VEngine {
    
    class Evaluate {
    public:
        static int evaluate(const Board& board);
        static int lazyEvaluate(const Board& board);
        
        // Compatibility with NNUE interface
        static bool load(const std::string& path);
        static const std::vector<float>& getBiases();
        static void computeLayer(float* input, float* weights, float* biases, float* output, int inSize, int outSize, bool activate);

    private:
        static void evaluatePawnStructure(const Board& board, int32_t& pawnMGDiff, int32_t& pawnEGDiff);
        static void evaluatePieces(const Board& board, int32_t mg[2], int32_t eg[2]);
    };

    extern const int32_t PieceValueMG[6];
    extern const int32_t PieceValueEG[6];
    extern const int32_t game_phase_increment[6];

    #define S(mg, eg) ((int32_t)(((uint32_t)(uint16_t)(eg) << 16) | (uint16_t)(mg)))
    inline int32_t unpack_mg(int32_t s) { return (int16_t)(s & 0xFFFF); }
    inline int32_t unpack_eg(int32_t s) { return (int16_t)((s >> 16) & 0xFFFF); }
}

#endif // VENGINE_EVAL_HCE_EVALUATOR_H
