#include "eval/hce/EvaluateConstants.hpp"
#include "eval/hce/Evaluate.hpp"
#include "core/Bitboard.hpp"
#include "core/Board.hpp"
#include "search/PawnTable.hpp"



namespace VEngine {

    PawnTable g_pawnTable(2);



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
