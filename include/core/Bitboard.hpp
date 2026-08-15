#ifndef CHESS_BITBOARD_H
#define CHESS_BITBOARD_H

#include "common.h"
#include "core/Constants.hpp"

namespace VEngine {





    class Bitboard {
    public:
        static uint64_t rookTable[64][4096];
        static uint64_t bishopTable[64][512];
        static uint64_t knightTable[64];
        static uint64_t kingTable[64];
        
        static const uint64_t rookMasks[64];
        static const uint64_t rookMagics[64];
        static const int rookShifts[64];

        static const uint64_t bishopMasks[64];
        static const uint64_t bishopMagics[64];
        static const int bishopShifts[64];

        static void init();

        static inline uint64_t getRookAttacks(int sq, uint64_t occupancy) {
            occupancy &= rookMasks[sq];
            unsigned idx = (occupancy * rookMagics[sq]) >> rookShifts[sq];
            return rookTable[sq][idx];
        }

        static inline uint64_t getBishopAttacks(int sq, uint64_t occupancy) {
            occupancy &= bishopMasks[sq];
            unsigned idx = (occupancy * bishopMagics[sq]) >> bishopShifts[sq];
            return bishopTable[sq][idx];
        }

        static inline uint64_t getQueenAttacks(int sq, uint64_t occupancy) {
            return getRookAttacks(sq, occupancy) | getBishopAttacks(sq, occupancy);
        }

        static inline uint64_t getKnightAttacks(int sq) { return knightTable[sq]; }
        static inline uint64_t getKingAttacks(int sq) { return kingTable[sq]; }
        static inline uint64_t getPawnAttacks(int sq, int color){
            uint64_t bb = (1ULL << sq);
            if (color == White) {
                return (((bb << 7) & 0x7F7F7F7F7F7F7F7FULL) | ((bb << 9) & 0xFEFEFEFEFEFEFEFEULL));
            } else {
                return (((bb >> 7) & 0xFEFEFEFEFEFEFEFEULL) | ((bb >> 9) & 0x7F7F7F7F7F7F7F7FULL));
            }
        };

        // Bit manipulation helpers
        static inline void setBit(uint64_t& bb, int sq) { bb |= (1ULL << sq); }
        static inline void popBit(uint64_t& bb, int sq) { bb &= ~(1ULL << sq); }
        static inline bool getBit(uint64_t bb, int sq) { return bb & (1ULL << sq); }
        static inline int countBits(uint64_t bb) { return __builtin_popcountll(bb); }
        static inline int getLsb(uint64_t bb) { return __builtin_ctzll(bb); }
        static inline void popLsb(uint64_t& bb) { bb &= (bb - 1); }

        static void print(uint64_t bb);
    };
}

#endif // CHESS_BITBOARD_H