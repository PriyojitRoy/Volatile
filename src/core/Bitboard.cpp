
#include "chess/core/Bitboard.hpp"

namespace ChessCore {

    uint64_t Bitboard::rookTable[64][4096];
    uint64_t Bitboard::bishopTable[64][512];
    uint64_t Bitboard::knightTable[64];
    uint64_t Bitboard::kingTable[64];

    const uint64_t Bitboard::rookMasks[64] = {
    0x101010101017eULL,
    0x202020202027cULL,
    0x404040404047aULL,
    0x8080808080876ULL,
    0x1010101010106eULL,
    0x2020202020205eULL,
    0x4040404040403eULL,
    0x8080808080807eULL,
    0x1010101017e00ULL,
    0x2020202027c00ULL,
    0x4040404047a00ULL,
    0x8080808087600ULL,
    0x10101010106e00ULL,
    0x20202020205e00ULL,
    0x40404040403e00ULL,
    0x80808080807e00ULL,
    0x10101017e0100ULL,
    0x20202027c0200ULL,
    0x40404047a0400ULL,
    0x8080808760800ULL,
    0x101010106e1000ULL,
    0x202020205e2000ULL,
    0x404040403e4000ULL,
    0x808080807e8000ULL,
    0x101017e010100ULL,
    0x202027c020200ULL,
    0x404047a040400ULL,
    0x8080876080800ULL,
    0x1010106e101000ULL,
    0x2020205e202000ULL,
    0x4040403e404000ULL,
    0x8080807e808000ULL,
    0x1017e01010100ULL,
    0x2027c02020200ULL,
    0x4047a04040400ULL,
    0x8087608080800ULL,
    0x10106e10101000ULL,
    0x20205e20202000ULL,
    0x40403e40404000ULL,
    0x80807e80808000ULL,
    0x17e0101010100ULL,
    0x27c0202020200ULL,
    0x47a0404040400ULL,
    0x8760808080800ULL,
    0x106e1010101000ULL,
    0x205e2020202000ULL,
    0x403e4040404000ULL,
    0x807e8080808000ULL,
    0x7e010101010100ULL,
    0x7c020202020200ULL,
    0x7a040404040400ULL,
    0x76080808080800ULL,
    0x6e101010101000ULL,
    0x5e202020202000ULL,
    0x3e404040404000ULL,
    0x7e808080808000ULL,
    0x7e01010101010100ULL,
    0x7c02020202020200ULL,
    0x7a04040404040400ULL,
    0x7608080808080800ULL,
    0x6e10101010101000ULL,
    0x5e20202020202000ULL,
    0x3e40404040404000ULL,
    0x7e80808080808000ULL,
};

const int Bitboard::rookShifts[64] = {
    52,
    53,
    53,
    53,
    53,
    53,
    53,
    52,
    53,
    54,
    54,
    54,
    54,
    54,
    54,
    53,
    53,
    54,
    54,
    54,
    54,
    54,
    54,
    53,
    53,
    54,
    54,
    54,
    54,
    54,
    54,
    53,
    53,
    54,
    54,
    54,
    54,
    54,
    54,
    53,
    53,
    54,
    54,
    54,
    54,
    54,
    54,
    53,
    53,
    54,
    54,
    54,
    54,
    54,
    54,
    53,
    52,
    53,
    53,
    53,
    53,
    53,
    53,
    52,
};

const uint64_t Bitboard::rookMagics[64] = {
    0x180001826400080ULL,
    0x8440004010042000ULL,
    0x3480088010012000ULL,
    0x200102042000804ULL,
    0x1600200200041108ULL,
    0x200010490084200ULL,
    0x8080008002000100ULL,
    0x20008210044820cULL,
    0x2320800840062080ULL,
    0x1008804004802000ULL,
    0x1420801000200080ULL,
    0x108a000842002010ULL,
    0x1101000800100502ULL,
    0x1096001884102200ULL,
    0x411000100220004ULL,
    0x1000802251800900ULL,
    0x80004000200050ULL,
    0x2050004040002008ULL,
    0x810022001240800ULL,
    0x2001010010000c21ULL,
    0x203050008011100ULL,
    0x4808004000200ULL,
    0x8900040002080110ULL,
    0x5205a0011a04304ULL,
    0x40008020800aULL,
    0x80200080804000ULL,
    0x100200100104100ULL,
    0x420080480100181ULL,
    0x11009100180114ULL,
    0x6104000202001008ULL,
    0x500a50026002cULL,
    0x801880004100ULL,
    0x400081c002800230ULL,
    0x5001201000c00042ULL,
    0x1c62008612002040ULL,
    0x100080800800ULL,
    0x808020040c01400ULL,
    0x4410020080800400ULL,
    0x800020804000110ULL,
    0x801040801100ULL,
    0x80204000908002ULL,
    0x2220201000414008ULL,
    0x2050108200420020ULL,
    0x2000100101090020ULL,
    0x4088020004004040ULL,
    0xd4040002008080ULL,
    0x2181001200050004ULL,
    0xa80a804400a20001ULL,
    0x524c210242920200ULL,
    0x408200802a4b0200ULL,
    0x202102001094100ULL,
    0x4100008210100ULL,
    0x1000040080080080ULL,
    0x102000204008080ULL,
    0x1009000600140d00ULL,
    0x4100842200ULL,
    0x18010020c2118001ULL,
    0x2042221284400101ULL,
    0x2002084100842ULL,
    0x83842009001001ULL,
    0x2003008208001005ULL,
    0x2000801441082ULL,
    0x102000104080082ULL,
    0x2130080440103a6ULL,
};

const uint64_t Bitboard::bishopMasks[64] = {
    0x40201008040200ULL,
    0x402010080400ULL,
    0x4020100a00ULL,
    0x40221400ULL,
    0x2442800ULL,
    0x204085000ULL,
    0x20408102000ULL,
    0x2040810204000ULL,
    0x20100804020000ULL,
    0x40201008040000ULL,
    0x4020100a0000ULL,
    0x4022140000ULL,
    0x244280000ULL,
    0x20408500000ULL,
    0x2040810200000ULL,
    0x4081020400000ULL,
    0x10080402000200ULL,
    0x20100804000400ULL,
    0x4020100a000a00ULL,
    0x402214001400ULL,
    0x24428002800ULL,
    0x2040850005000ULL,
    0x4081020002000ULL,
    0x8102040004000ULL,
    0x8040200020400ULL,
    0x10080400040800ULL,
    0x20100a000a1000ULL,
    0x40221400142200ULL,
    0x2442800284400ULL,
    0x4085000500800ULL,
    0x8102000201000ULL,
    0x10204000402000ULL,
    0x4020002040800ULL,
    0x8040004081000ULL,
    0x100a000a102000ULL,
    0x22140014224000ULL,
    0x44280028440200ULL,
    0x8500050080400ULL,
    0x10200020100800ULL,
    0x20400040201000ULL,
    0x2000204081000ULL,
    0x4000408102000ULL,
    0xa000a10204000ULL,
    0x14001422400000ULL,
    0x28002844020000ULL,
    0x50005008040200ULL,
    0x20002010080400ULL,
    0x40004020100800ULL,
    0x20408102000ULL,
    0x40810204000ULL,
    0xa1020400000ULL,
    0x142240000000ULL,
    0x284402000000ULL,
    0x500804020000ULL,
    0x201008040200ULL,
    0x402010080400ULL,
    0x2040810204000ULL,
    0x4081020400000ULL,
    0xa102040000000ULL,
    0x14224000000000ULL,
    0x28440200000000ULL,
    0x50080402000000ULL,
    0x20100804020000ULL,
    0x40201008040200ULL,
};

const int Bitboard::bishopShifts[64] = {
    58,
    59,
    59,
    59,
    59,
    59,
    59,
    58,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    57,
    57,
    57,
    57,
    59,
    59,
    59,
    59,
    57,
    55,
    55,
    57,
    59,
    59,
    59,
    59,
    57,
    55,
    55,
    57,
    59,
    59,
    59,
    59,
    57,
    57,
    57,
    57,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    59,
    58,
    59,
    59,
    59,
    59,
    59,
    59,
    58,
};

const uint64_t Bitboard::bishopMagics[64] = {
    0x342d03401040224ULL,
    0x20480091024508ULL,
    0xc1280a1410270280ULL,
    0x4002408100000240ULL,
    0x114102890114000ULL,
    0x101010840001028ULL,
    0x410c00c804100000ULL,
    0x5002020104020280ULL,
    0x100400408063240ULL,
    0x828100401040030ULL,
    0x2c44904050000ULL,
    0x942020a02062000ULL,
    0x180142420600008ULL,
    0x412009010082020ULL,
    0x40424044486ULL,
    0x1800010041300800ULL,
    0x510002204100080ULL,
    0x21200208210100ULL,
    0x18013000404009ULL,
    0x140a024c40110004ULL,
    0x34000200940081ULL,
    0x2010022100200ULL,
    0x104000100a21081ULL,
    0x1000384008a02ULL,
    0x208404420040100ULL,
    0x1810041010810201ULL,
    0x40022c0008080023ULL,
    0x421004014040002ULL,
    0xe044040040410040ULL,
    0x6110184020805000ULL,
    0x101220004008458ULL,
    0x1001002001040100ULL,
    0x5850118410680810ULL,
    0x48510800041840ULL,
    0x20050000c4101ULL,
    0xc0820082180080ULL,
    0x1030020200012008ULL,
    0x5020043005200b0ULL,
    0x2201010104022804ULL,
    0x4008840280010085ULL,
    0x824820004100ULL,
    0x102200a404006020ULL,
    0x3000201450000800ULL,
    0x8280044010440204ULL,
    0x406040408200400ULL,
    0x42504202008420ULL,
    0x4044100212100040ULL,
    0x88220040480200ULL,
    0x20411010110800ULL,
    0x440210110100010ULL,
    0x8002120044120c05ULL,
    0x86040120a80000ULL,
    0x2000015002088080ULL,
    0x971202430408000ULL,
    0x4141080821004c00ULL,
    0x610020821002100ULL,
    0x8220818882800ULL,
    0x1004008884104200ULL,
    0x8010100a80402ULL,
    0x804c000420220ULL,
    0x2001c12020202ULL,
    0x4000804008010440ULL,
    0x441050404181200ULL,
    0x221081000458024ULL,
};


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
            uint64_t mask = rookMasks[sq];
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
                int idx = (int)((occ * rookMagics[sq]) >> rookShifts[sq]);
                rookTable[sq][idx] = generateRookAttacksSlow(sq, occ);
            }
        }

        for (int sq = 0; sq < 64; sq++) {
            uint64_t mask = bishopMasks[sq];
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
                int idx = (int)((occ * bishopMagics[sq]) >> bishopShifts[sq]);
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