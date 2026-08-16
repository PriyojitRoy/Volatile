#ifndef VENGINE_CONSTANT_ARRAYS_H
#define VENGINE_CONSTANT_ARRAYS_H

#include <cstdint>

namespace VEngine {
    class Bitboard;
    class OpeningBook;

    struct ConstantArrays {
        class BitboardData {
        private:
            friend class Bitboard;
            static const uint64_t rookMasks[64];
            static const int rookShifts[64];
            static const uint64_t rookMagics[64];
            
            static const uint64_t bishopMasks[64];
            static const int bishopShifts[64];
            static const uint64_t bishopMagics[64];
        };

        class PolyglotData {
        private:
            friend class OpeningBook;
            static const uint64_t PolyglotRandoms[781];
        };
    };
}

#endif // VENGINE_CONSTANT_ARRAYS_H
