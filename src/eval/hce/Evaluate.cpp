#include "eval/hce/Evaluate.hpp"
#include "core/Bitboard.hpp"
#include "core/Board.hpp"
#include "search/hce/PawnTable.hpp"



namespace VEngine {

    PawnTable g_pawnTable(2);

    const int32_t PSQT[6][64] = {
        {
            S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
            S(47, 178), S(76, 190), S(72, 176), S(81, 142), S(80, 119), S(80, 128), S(49, 149), S(33, 158),
            S(87, 159), S(95, 179), S(93, 165), S(104, 145), S(114, 107), S(156, 113), S(133, 156), S(105, 135),
            S(79, 127), S(93, 134), S(92, 123), S(77, 114), S(93, 101), S(96, 103), S(82, 119), S(78, 99),
            S(73, 103), S(78, 117), S(85, 103), S(83, 112), S(77, 101), S(86, 94), S(69, 106), S(70, 84),
            S(89, 97), S(99, 107), S(90, 104), S(64, 113), S(82, 110), S(87, 102), S(94, 101), S(81, 83),
            S(84, 99), S(113, 103), S(102, 100), S(83, 108), S(76, 122), S(96, 103), S(105, 100), S(76, 79),
            S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
        },
        {
            S(13, 81), S(19, 142), S(60, 130), S(89, 121), S(123, 118), S(50, 103), S(44, 137), S(63, 58),
            S(100, 143), S(106, 126), S(84, 125), S(103, 126), S(98, 116), S(120, 107), S(109, 119), S(126, 125),
            S(110, 119), S(104, 123), S(155, 168), S(162, 170), S(174, 163), S(196, 148), S(112, 118), S(127, 111),
            S(120, 133), S(106, 134), S(154, 177), S(171, 181), S(153, 181), S(175, 178), S(109, 138), S(147, 123),
            S(107, 134), S(95, 129), S(137, 178), S(136, 180), S(144, 184), S(142, 169), S(114, 126), S(118, 124),
            S(91, 120), S(82, 125), S(121, 161), S(128, 174), S(139, 171), S(127, 156), S(99, 117), S(108, 119),
            S(103, 144), S(90, 126), S(74, 123), S(89, 123), S(88, 120), S(83, 118), S(103, 115), S(128, 152),
            S(86, 137), S(114, 136), S(81, 123), S(93, 122), S(97, 123), S(101, 113), S(115, 143), S(114, 125)
        },
        {
            S(111, 171), S(90, 176), S(95, 167), S(57, 178), S(74, 173), S(81, 166), S(115, 163), S(80, 166),
            S(104, 163), S(116, 164), S(114, 163), S(107, 162), S(108, 157), S(119, 159), S(98, 171), S(103, 163),
            S(124, 175), S(137, 166), S(131, 164), S(133, 158), S(128, 163), S(151, 167), S(141, 166), S(130, 177),
            S(115, 172), S(121, 170), S(125, 167), S(142, 175), S(133, 169), S(129, 170), S(121, 166), S(118, 168),
            S(121, 169), S(110, 172), S(123, 168), S(135, 171), S(134, 167), S(129, 163), S(121, 165), S(129, 159),
            S(123, 169), S(135, 165), S(131, 165), S(130, 165), S(133, 168), S(135, 161), S(138, 157), S(143, 158),
            S(142, 168), S(138, 154), S(140, 151), S(128, 160), S(135, 159), S(138, 155), S(153, 158), S(145, 152),
            S(135, 158), S(145, 164), S(134, 160), S(123, 165), S(128, 163), S(126, 171), S(134, 157), S(153, 140)
        },
        {
            S(160, 305), S(152, 309), S(156, 317), S(155, 315), S(158, 307), S(181, 302), S(162, 306), S(183, 301),
            S(157, 301), S(159, 307), S(172, 313), S(188, 304), S(173, 306), S(198, 291), S(193, 288), S(206, 282),
            S(151, 296), S(176, 292), S(170, 296), S(173, 290), S(197, 282), S(205, 274), S(226, 271), S(197, 274),
            S(150, 299), S(161, 294), S(163, 299), S(170, 295), S(165, 287), S(177, 278), S(172, 282), S(172, 279),
            S(137, 298), S(133, 300), S(142, 299), S(152, 298), S(149, 297), S(143, 291), S(152, 284), S(148, 283),
            S(132, 299), S(132, 295), S(138, 295), S(143, 298), S(145, 294), S(148, 284), S(162, 271), S(150, 276),
            S(136, 295), S(140, 295), S(150, 295), S(152, 293), S(154, 288), S(154, 283), S(166, 274), S(144, 283),
            S(154, 302), S(151, 297), S(153, 302), S(159, 295), S(159, 291), S(150, 296), S(158, 287), S(156, 288)
        },
        {
            S(327, 567), S(320, 573), S(338, 588), S(365, 573), S(348, 576), S(360, 574), S(406, 525), S(360, 560),
            S(348, 553), S(334, 564), S(331, 595), S(319, 613), S(311, 631), S(358, 575), S(362, 569), S(389, 573),
            S(358, 552), S(351, 557), S(353, 578), S(350, 585), S(358, 588), S(383, 575), S(392, 551), S(378, 565),
            S(341, 570), S(349, 570), S(344, 576), S(343, 590), S(343, 592), S(350, 584), S(359, 586), S(357, 578),
            S(350, 562), S(339, 576), S(343, 572), S(347, 584), S(344, 581), S(346, 573), S(352, 568), S(356, 573),
            S(346, 547), S(353, 555), S(346, 567), S(343, 563), S(345, 568), S(348, 562), S(361, 543), S(356, 552),
            S(349, 551), S(352, 546), S(359, 540), S(354, 551), S(354, 552), S(353, 525), S(360, 505), S(373, 499),
            S(344, 554), S(345, 545), S(348, 546), S(353, 558), S(351, 537), S(330, 536), S(342, 526), S(355, 523)
        },
        {
            S(75, -119), S(73, -69), S(102, -52), S(-16, -8), S(-39, 4), S(-30, 5), S(41, -12), S(157, -113),
            S(-59, -28), S(43, -7), S(22, 9), S(109, 1), S(22, 39), S(16, 47), S(31, 30), S(-19, 8),
            S(-77, -19), S(78, -6), S(38, 20), S(20, 36), S(23, 60), S(60, 45), S(27, 32), S(-36, 10),
            S(-44, -27), S(-2, 0), S(-5, 25), S(-43, 49), S(-69, 76), S(-49, 60), S(-64, 38), S(-124, 22),
            S(-57, -30), S(-11, -6), S(-22, 20), S(-49, 41), S(-69, 73), S(-38, 50), S(-56, 32), S(-130, 21),
            S(-26, -34), S(21, -14), S(-6, 7), S(-15, 19), S(-24, 52), S(-18, 39), S(-1, 19), S(-51, 9),
            S(29, -45), S(31, -21), S(22, -9), S(-1, 0), S(-18, 35), S(-4, 26), S(25, 7), S(5, -9),
            S(13, -80), S(43, -60), S(25, -39), S(-38, -21), S(-5, -5), S(-33, 6), S(16, -17), S(0, -44)
        },
    };

    const int32_t mobilities[5][28] = {
        { S(0, 0), S(0, 0), S(115, 196), S(140, 193), S(163, 225), S(0, 0), S(198, 238), S(0, 0), S(175, 208), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0) },
        { S(0, 0), S(122, 144), S(136, 139), S(140, 169), S(154, 179), S(158, 188), S(169, 203), S(174, 210), S(180, 218), S(183, 222), S(186, 227), S(186, 225), S(188, 223), S(206, 220), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0) },
        { S(0, 0), S(0, 0), S(196, 305), S(201, 322), S(205, 330), S(208, 336), S(208, 340), S(208, 346), S(211, 348), S(214, 351), S(215, 356), S(217, 360), S(218, 366), S(222, 370), S(220, 375), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0) },
        { S(0, 0), S(0, 0), S(0, 0), S(442, 344), S(433, 424), S(460, 471), S(462, 494), S(458, 556), S(460, 571), S(458, 593), S(460, 600), S(461, 612), S(462, 623), S(464, 626), S(467, 630), S(467, 638), S(466, 645), S(468, 650), S(466, 656), S(467, 660), S(467, 668), S(469, 668), S(471, 668), S(479, 663), S(476, 665), S(491, 659), S(525, 638), S(574, 611) },
        { S(0, 0), S(0, 0), S(0, 0), S(48, -7), S(53, -16), S(37, 2), S(27, 0), S(24, -5), S(19, -3), S(15, -3), S(14, -3), S(6, 0), S(4, -1), S(-3, 1), S(-12, 3), S(-22, 5), S(-36, 6), S(-51, 6), S(-63, 7), S(-79, 6), S(-79, 2), S(-84, 0), S(-91, -2), S(-98, -7), S(-114, -7), S(-98, -19), S(-112, -17), S(-90, -30) },
    };

    const int32_t bishop_pair = S(19, 59);

    const int32_t passed_pawns[64] = {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(47, 178), S(76, 190), S(72, 176), S(81, 142), S(80, 119), S(80, 128), S(49, 149), S(33, 158),
        S(16, 142), S(30, 143), S(21, 106), S(8, 72), S(12, 83), S(11, 97), S(-20, 106), S(-41, 140),
        S(14, 79), S(19, 76), S(19, 52), S(13, 47), S(2, 46), S(10, 52), S(0, 69), S(-11, 78),
        S(5, 45), S(1, 39), S(-9, 30), S(-2, 23), S(-11, 26), S(-5, 30), S(3, 42), S(0, 40),
        S(0, 9), S(-8, 18), S(-17, 13), S(-17, 8), S(-12, 7), S(-6, 6), S(8, 22), S(14, 7),
        S(-7, 7), S(-3, 11), S(-16, 12), S(-14, 6), S(1, -8), S(1, -2), S(19, 2), S(7, 7),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    };

    const int32_t inner_king_zone_attacks[4] = { S(10, -6), S(18, -4), S(20, -7), S(13, 7) };
    const int32_t outer_king_zone_attacks[4] = { S(0, 1), S(0, 0), S(4, -4), S(2, 1) };
    const int32_t doubled_pawn_penalty[8] = { S(-7, -39), S(-4, -26), S(-11, -24), S(-10, -10), S(-9, -19), S(-12, -22), S(-5, -24), S(-14, -34) };

    const int32_t pawn_storm[64] = {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(22, -59), S(-34, -53), S(-6, -92), S(-16, -48), S(-60, 48), S(-48, -10), S(-3, -10), S(-33, 0),
        S(-19, -33), S(-20, -41), S(10, -51), S(-1, -27), S(-10, 17), S(-30, 0), S(-44, -5), S(-44, 5),
        S(-30, -10), S(-29, -13), S(-22, -13), S(-3, -14), S(-8, 13), S(-20, 5), S(-34, 11), S(-47, 17),
        S(-32, -5), S(-25, -4), S(-22, -2), S(-8, -8), S(-5, 6), S(-23, 7), S(-28, 8), S(-43, 16),
        S(-45, 3), S(-44, 4), S(-23, 0), S(7, -5), S(-11, 1), S(-23, 3), S(-57, 17), S(-58, 23),
        S(-37, 8), S(-56, 10), S(-45, 9), S(-23, 3), S(-27, 4), S(-39, 8), S(-71, 27), S(-53, 37),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    };

    const int32_t isolated_pawns[64] = {
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0),
        S(44, -15), S(107, -93), S(47, 0), S(49, -1), S(16, 5), S(1, 28), S(-28, 27), S(-2, 4),
        S(11, -16), S(12, -41), S(1, -16), S(3, -5), S(1, -11), S(-5, -3), S(22, -21), S(8, -22),
        S(10, -8), S(1, -24), S(-5, -15), S(-6, -18), S(-5, -20), S(8, -17), S(22, -24), S(8, -6),
        S(0, 2), S(-1, -11), S(-16, -6), S(-18, -19), S(-12, -18), S(-12, -5), S(-10, -7), S(-7, 3),
        S(-12, -2), S(-12, -15), S(-23, -11), S(-15, -17), S(-30, -16), S(-21, -8), S(-22, -15), S(-21, 0),
        S(-10, -6), S(-16, -10), S(-5, -13), S(-28, -7), S(-28, -12), S(-10, -7), S(-4, -19), S(-13, 1),
        S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0), S(0, 0)
    };

    const int32_t threats[6][6] = {
        { S(3, -10), S(12, -44), S(26, -30), S(-19, -27), S(-9, -62), S(-90, 2) },
        { S(-11, 4), S(0, 0), S(20, 22), S(48, -5), S(22, -34), S(0, 0) },
        { S(-6, 5), S(15, 19), S(0, 0), S(30, 4), S(33, 68), S(0, 0) },
        { S(-17, 12), S(0, 14), S(9, 11), S(0, 0), S(45, 1), S(0, 0) },
        { S(-4, 5), S(0, 6), S(-3, 26), S(2, -1), S(0, 0), S(0, 0) },
        { S(36, 31), S(-41, 8), S(-19, 21), S(-15, -1), S(0, 0), S(0, 0) },
    };

    const int32_t rook_semi_open[2] = { S(22, 9), S(55, 4) };
    const int32_t phalanx_pawns[8] = { S(0, 0), S(-2, -7), S(-1, -3), S(18, 11), S(47, 39), S(128, 167), S(-129, 412), S(0, 0) };
    const int32_t PieceValueMG[6] = { 90, 320, 330, 500, 920, 0 };
    const int32_t PieceValueEG[6] = { 110, 290, 340, 620, 1000, 0 };
    const int32_t game_phase_increment[6] = { 0, 1, 1, 2, 4, 0 };

    struct EvalEntry {
        uint64_t key;
        int32_t score;
    };
    
    const int EVAL_HASH_SIZE = 1048576;
    EvalEntry evalTable[EVAL_HASH_SIZE] = {};

    bool Evaluate::load(const std::string&) { return true; }
    const std::vector<float>& Evaluate::getBiases() { static std::vector<float> v; return v; }
    void Evaluate::computeLayer(float*, float*, float*, float*, int, int, bool) {}

    void Evaluate::evaluatePawnStructure(const Board& board, int32_t& pawnMGDiff, int32_t& pawnEGDiff) {
        int packedScore;
        if (g_pawnTable.probe(board.pawnKey, packedScore)) {
            pawnMGDiff = unpack_mg(packedScore);
            pawnEGDiff = unpack_eg(packedScore);
            return;
        }

        int32_t pawnMG[2] = {0, 0};
        int32_t pawnEG[2] = {0, 0};
        uint64_t pawns[2] = { board.getPieces(Pawn, White), board.getPieces(Pawn, Black) };

        for (int c = White; c <= Black; c++) {
            int them = 1 - c;
            uint64_t bb = pawns[c];
            while(bb) {
                int sq = Bitboard::getLsb(bb);
                Bitboard::popLsb(bb);
                int tableSq = (c == White) ? (sq ^ 56) : sq;
                uint64_t fileMask = 0x0101010101010101ULL << (sq % 8);
                uint64_t adjMask = ((fileMask << 1) & ~0x0101010101010101ULL) | ((fileMask >> 1) & ~0x8080808080808080ULL);
                uint64_t forwardMask = (c == White) ? ~((1ULL << (8 * (sq / 8 + 1))) - 1) : (1ULL << (8 * (sq / 8))) - 1;

                if (pawns[c] & fileMask & forwardMask) {
                    int32_t raw = doubled_pawn_penalty[(c == White) ? (7 - (sq % 8)) : (sq % 8)];
                    pawnMG[c] += unpack_mg(raw); pawnEG[c] += unpack_eg(raw);
                }
                if ((pawns[them] & (fileMask | adjMask) & forwardMask) == 0) {
                    int32_t raw = passed_pawns[tableSq];
                    pawnMG[c] += unpack_mg(raw); pawnEG[c] += unpack_eg(raw);
                }
                if ((pawns[c] & adjMask) == 0) {
                    int32_t raw = isolated_pawns[tableSq];
                    pawnMG[c] += unpack_mg(raw); pawnEG[c] += unpack_eg(raw);
                }
                if ((sq % 8 != 0) && (pawns[c] & (1ULL << (sq - 1)))) {
                    int rank = sq / 8;
                    int32_t raw = phalanx_pawns[(c == White) ? rank : (7 - rank)];
                    pawnMG[c] += unpack_mg(raw); pawnEG[c] += unpack_eg(raw);
                }
            }
        }
        pawnMGDiff = pawnMG[White] - pawnMG[Black];
        pawnEGDiff = pawnEG[White] - pawnEG[Black];
        g_pawnTable.store(board.pawnKey, S(pawnMGDiff, pawnEGDiff));
    }

    void Evaluate::evaluatePieces(const Board& board, int32_t mg[2], int32_t eg[2]) {
        uint64_t occ = board.getAllPieces();
        uint64_t pieces[2][6];
        for (int c = White; c <= Black; c++) {
            for (int p = Pawn; p <= King; p++) pieces[c][p] = board.getPieces(p, c);
        }

        int kingSq[2] = { Bitboard::getLsb(pieces[White][King]), Bitboard::getLsb(pieces[Black][King]) };
        uint64_t kingInner[2], kingOuter[2];
        for (int c = White; c <= Black; c++) {
            kingInner[c] = Bitboard::getKingAttacks(kingSq[c]);
            uint64_t ring = kingInner[c];
            ring |= (ring << 8) | (ring >> 8);
            ring |= ((ring & ~0x0101010101010101ULL) >> 1) | ((ring & ~0x8080808080808080ULL) << 1);
            kingOuter[c] = ring & ~kingInner[c] & ~(1ULL << kingSq[c]);
        }

        uint64_t stormMask[2] = { (kingSq[White] % 8 < 4) ? 0xF0F0F0F0F0F0F0F0ULL : 0x0F0F0F0F0F0F0F0FULL, (kingSq[Black] % 8 < 4) ? 0xF0F0F0F0F0F0F0F0ULL : 0x0F0F0F0F0F0F0F0FULL };
        int rooksSemiOpen[2] = {0, 0};

        for (int c = White; c <= Black; c++) {
            int them = 1 - c;
            uint64_t enemyOcc = board.occupancy[them];
            for (int p = Pawn; p <= King; p++) {
                uint64_t bb = pieces[c][p];
                while (bb) {
                    int sq = Bitboard::getLsb(bb);
                    Bitboard::popLsb(bb);
                    uint64_t attacks = 0;
                    if (p > Pawn) {
                        if (p == Knight) attacks = Bitboard::getKnightAttacks(sq);
                        else if (p == Bishop) attacks = Bitboard::getBishopAttacks(sq, occ);
                        else if (p == Rook) attacks = Bitboard::getRookAttacks(sq, occ);
                        else if (p == Queen) attacks = Bitboard::getBishopAttacks(sq, occ) | Bitboard::getRookAttacks(sq, occ);
                        else if (p == King) attacks = Bitboard::getKingAttacks(sq);

                        int mob = std::min(27, Bitboard::countBits(attacks));
                        int32_t raw_mob = mobilities[p - 1][mob];
                        mg[c] += unpack_mg(raw_mob); eg[c] += unpack_eg(raw_mob);

                        if (p != King) {
                            if (uint64_t innerAttacks = attacks & kingInner[them]) {
                                int inner_cnt = Bitboard::countBits(innerAttacks);
                                mg[c] += unpack_mg(inner_king_zone_attacks[p - 1]) * inner_cnt;
                                eg[c] += unpack_eg(inner_king_zone_attacks[p - 1]) * inner_cnt;
                            }
                            if (uint64_t outerAttacks = attacks & kingOuter[them]) {
                                int outer_cnt = Bitboard::countBits(outerAttacks);
                                mg[c] += unpack_mg(outer_king_zone_attacks[p - 1]) * outer_cnt;
                                eg[c] += unpack_eg(outer_king_zone_attacks[p - 1]) * outer_cnt;
                            }
                        }
                        if (p == Rook) {
                            uint64_t fileMask = 0x0101010101010101ULL << (sq % 8);
                            if ((pieces[c][Pawn] & fileMask) == 0) rooksSemiOpen[c]++;
                        }
                    } else {
                        if ((1ULL << sq) & stormMask[c]) {
                            int tableSq = (c == White) ? (sq ^ 56) : sq;
                            int32_t raw_storm = pawn_storm[tableSq];
                            mg[c] += unpack_mg(raw_storm); eg[c] += unpack_eg(raw_storm);
                        }
                        attacks = Bitboard::getPawnAttacks(sq, c);
                    }

                    uint64_t activeAttacks = attacks & enemyOcc;
                    if (activeAttacks) {
                        if (uint64_t b = activeAttacks & pieces[them][Pawn]) { int cnt = Bitboard::countBits(b); mg[c] += unpack_mg(threats[p][0]) * cnt; eg[c] += unpack_eg(threats[p][0]) * cnt; }
                        if (uint64_t b = activeAttacks & pieces[them][Knight]) { int cnt = Bitboard::countBits(b); mg[c] += unpack_mg(threats[p][1]) * cnt; eg[c] += unpack_eg(threats[p][1]) * cnt; }
                        if (uint64_t b = activeAttacks & pieces[them][Bishop]) { int cnt = Bitboard::countBits(b); mg[c] += unpack_mg(threats[p][2]) * cnt; eg[c] += unpack_eg(threats[p][2]) * cnt; }
                        if (uint64_t b = activeAttacks & pieces[them][Rook]) { int cnt = Bitboard::countBits(b); mg[c] += unpack_mg(threats[p][3]) * cnt; eg[c] += unpack_eg(threats[p][3]) * cnt; }
                        if (uint64_t b = activeAttacks & pieces[them][Queen]) { int cnt = Bitboard::countBits(b); mg[c] += unpack_mg(threats[p][4]) * cnt; eg[c] += unpack_eg(threats[p][4]) * cnt; }
                        if (uint64_t b = activeAttacks & pieces[them][King]) { int cnt = Bitboard::countBits(b); mg[c] += unpack_mg(threats[p][5]) * cnt; eg[c] += unpack_eg(threats[p][5]) * cnt; }
                    }
                }
            }
        }

        if (Bitboard::countBits(pieces[White][Bishop]) >= 2) { mg[White] += unpack_mg(bishop_pair); eg[White] += unpack_eg(bishop_pair); }
        if (Bitboard::countBits(pieces[Black][Bishop]) >= 2) { mg[Black] += unpack_mg(bishop_pair); eg[Black] += unpack_eg(bishop_pair); }
        if (rooksSemiOpen[White] == 1) { mg[White] += unpack_mg(rook_semi_open[0]); eg[White] += unpack_eg(rook_semi_open[0]); }
        if (rooksSemiOpen[White] >= 2) { mg[White] += unpack_mg(rook_semi_open[1]); eg[White] += unpack_eg(rook_semi_open[1]); }
        if (rooksSemiOpen[Black] == 1) { mg[Black] += unpack_mg(rook_semi_open[0]); eg[Black] += unpack_eg(rook_semi_open[0]); }
        if (rooksSemiOpen[Black] >= 2) { mg[Black] += unpack_mg(rook_semi_open[1]); eg[Black] += unpack_eg(rook_semi_open[1]); }
    }

    int Evaluate::evaluate(const Board& board) {
        uint64_t key = board.zorbitKey;
        int idx = key % EVAL_HASH_SIZE;
        if (evalTable[idx].key == key) return evalTable[idx].score;

        int32_t mg[2] = { board.evalState.mg[White], board.evalState.mg[Black] };
        int32_t eg[2] = { board.evalState.eg[White], board.evalState.eg[Black] };
        int phase = board.evalState.phase;
        int32_t pawnMGDiff = 0, pawnEGDiff = 0;
        evaluatePawnStructure(board, pawnMGDiff, pawnEGDiff);
        evaluatePieces(board, mg, eg);

        int stm = board.sideToMove;
        int32_t mg_score = mg[stm] - mg[1 - stm];
        int32_t eg_score = eg[stm] - eg[1 - stm];
        mg_score += (stm == White) ? pawnMGDiff : -pawnMGDiff;
        eg_score += (stm == White) ? pawnEGDiff : -pawnEGDiff;
        if (phase > 24) phase = 24;
        int32_t eg_phase = 24 - phase;
        int32_t finalScore = (mg_score * phase + eg_score * eg_phase) / 24;
        evalTable[idx].key = key;
        evalTable[idx].score = finalScore;
        return finalScore;
    }

    int Evaluate::lazyEvaluate(const Board& board) {
        int32_t mg[2] = { board.evalState.mg[White], board.evalState.mg[Black] };
        int32_t eg[2] = { board.evalState.eg[White], board.evalState.eg[Black] };
        int phase = board.evalState.phase;
        int32_t pawnMGDiff = 0, pawnEGDiff = 0;
        evaluatePawnStructure(board, pawnMGDiff, pawnEGDiff);
        int stm = board.sideToMove;
        int32_t mg_score = mg[stm] - mg[1 - stm];
        int32_t eg_score = eg[stm] - eg[1 - stm];
        mg_score += (stm == White) ? pawnMGDiff : -pawnMGDiff;
        eg_score += (stm == White) ? pawnEGDiff : -pawnEGDiff;
        if (phase > 24) phase = 24;
        int32_t eg_phase = 24 - phase;
        return (mg_score * phase + eg_score * eg_phase) / 24;
    }
}
