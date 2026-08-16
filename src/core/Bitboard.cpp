
#include "core/Bitboard.hpp"

namespace VEngine {

    uint64_t Bitboard::rookTable[64][4096];
    uint64_t Bitboard::bishopTable[64][512];
    uint64_t Bitboard::knightTable[64];
    uint64_t Bitboard::kingTable[64];








    uint64_t generateKnightAttacks(int sq) {
        uint64_t attacks = 0ULL;
        uint64_t bb = (1ULL << sq);
        
        if ((bb << 17) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bb << 17);
        if ((bb << 15) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bb << 15);
        if ((bb << 10) & 0xFCFCFCFCFCFCFCFCULL) attacks |= (bb << 10);
        if ((bb << 6)  & 0x3F3F3F3F3F3F3F3FULL) attacks |= (bb << 6);
        
        if ((bb >> 17) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bb >> 17);
        if ((bb >> 15) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bb >> 15);
        if ((bb >> 10) & 0x3F3F3F3F3F3F3F3FULL) attacks |= (bb >> 10);
        if ((bb >> 6)  & 0xFCFCFCFCFCFCFCFCULL) attacks |= (bb >> 6);
        
        return attacks;
    }

    uint64_t generateKingAttacks(int sq) {
        uint64_t attacks = 0ULL;
        uint64_t bb = (1ULL << sq);

        if (bb << 8) attacks |= (bb << 8); // North
        if (bb >> 8) attacks |= (bb >> 8); // South
        if ((bb << 1) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bb << 1); // East
        if ((bb >> 1) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bb >> 1); // West
        
        if ((bb << 9) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bb << 9); // NE
        if ((bb << 7) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bb << 7); // NW
        if ((bb >> 7) & 0xFEFEFEFEFEFEFEFEULL) attacks |= (bb >> 7); // SE
        if ((bb >> 9) & 0x7F7F7F7F7F7F7F7FULL) attacks |= (bb >> 9); // SW
        
        return attacks;
    }

    uint64_t generateRookAttacksSlow(int sq, uint64_t occ) {
        uint64_t attacks = 0ULL;
        int r = sq / 8;
        int f = sq % 8;

        for (int i = r + 1; i < 8; i++) {
            attacks |= (1ULL << (i * 8 + f));
            if (occ & (1ULL << (i * 8 + f))) break;
        }
        for (int i = r - 1; i >= 0; i--) {
            attacks |= (1ULL << (i * 8 + f));
            if (occ & (1ULL << (i * 8 + f))) break;
        }
        for (int i = f + 1; i < 8; i++) {
            attacks |= (1ULL << (r * 8 + i));
            if (occ & (1ULL << (r * 8 + i))) break;
        }
        for (int i = f - 1; i >= 0; i--) {
            attacks |= (1ULL << (r * 8 + i));
            if (occ & (1ULL << (r * 8 + i))) break;
        }
        return attacks;
    }

    uint64_t generateBishopAttacksSlow(int sq, uint64_t occ) {
        uint64_t attacks = 0ULL;
        int r = sq / 8;
        int f = sq % 8;

        for (int i = r + 1, j = f + 1; i < 8 && j < 8; i++, j++) {
            attacks |= (1ULL << (i * 8 + j));
            if (occ & (1ULL << (i * 8 + j))) break;
        }
        for (int i = r - 1, j = f + 1; i >= 0 && j < 8; i--, j++) {
            attacks |= (1ULL << (i * 8 + j));
            if (occ & (1ULL << (i * 8 + j))) break;
        }
        for (int i = r + 1, j = f - 1; i < 8 && j >= 0; i++, j--) {
            attacks |= (1ULL << (i * 8 + j));
            if (occ & (1ULL << (i * 8 + j))) break;
        }
        for (int i = r - 1, j = f - 1; i >= 0 && j >= 0; i--, j--) {
            attacks |= (1ULL << (i * 8 + j));
            if (occ & (1ULL << (i * 8 + j))) break;
        }
        return attacks;
    }

  
    void Bitboard::init() {
        std::cout << "Initializing Bitboards..." << std::endl;

        for (int sq = 0; sq < 64; sq++) {
            knightTable[sq] = generateKnightAttacks(sq);
            kingTable[sq] = generateKingAttacks(sq);
        }

        for (int sq = 0; sq < 64; sq++) {
            uint64_t mask = ConstantArrays::BitboardData::rookMasks[sq];
            int bits = countBits(mask); 
            int variations = 1 << bits;
            
            for (int i = 0; i < variations; i++) {
                uint64_t occ = 0ULL;
                int b = 0;
                for(int bitIdx = 0; bitIdx < 64; bitIdx++) {
                    if(mask & (1ULL << bitIdx)) {
                        if(i & (1 << b)) occ |= (1ULL << bitIdx);
                        b++;
                    }
                }
                int idx = (int)((occ * ConstantArrays::BitboardData::rookMagics[sq]) >> ConstantArrays::BitboardData::rookShifts[sq]);
                rookTable[sq][idx] = generateRookAttacksSlow(sq, occ);
            }
        }

        for (int sq = 0; sq < 64; sq++) {
            uint64_t mask = ConstantArrays::BitboardData::bishopMasks[sq];
            int bits = countBits(mask);
            int variations = 1 << bits;
            
            for (int i = 0; i < variations; i++) {
                uint64_t occ = 0ULL;
                int b = 0;
                for(int bitIdx = 0; bitIdx < 64; bitIdx++) {
                    if(mask & (1ULL << bitIdx)) {
                        if(i & (1 << b)) occ |= (1ULL << bitIdx);
                        b++;
                    }
                }
                int idx = (int)((occ * ConstantArrays::BitboardData::bishopMagics[sq]) >> ConstantArrays::BitboardData::bishopShifts[sq]);
                bishopTable[sq][idx] = generateBishopAttacksSlow(sq, occ);
            }
        }
        std::cout << "Bitboards Initialized!" << std::endl;
    }

    void Bitboard::print(uint64_t bb) {
        for (int r = 7; r >= 0; r--) {
            std::cout << r + 1 << " ";
            for (int f = 0; f < 8; f++) {
                int sq = r * 8 + f;
                std::cout << (getBit(bb, sq) ? "1 " : ". ");
            }
            std::cout << std::endl;
        }
        std::cout << "  a b c d e f g h" << std::endl << std::endl;
    }
}