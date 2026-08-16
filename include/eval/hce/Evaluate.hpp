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
}

#endif // VENGINE_EVAL_HCE_EVALUATOR_H
