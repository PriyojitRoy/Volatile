#include "search/Search.hpp"
#include "core/MoveGen.hpp"
#include "search/TT.hpp"
#include "core/Bitboard.hpp"
#ifdef USE_NNUE
#include "eval/nnue/Network.hpp"
namespace VEngine { using EvalBackend = Network; }
#else
#include "eval/hce/Evaluate.hpp"
namespace VEngine { using EvalBackend = Evaluate; }
#endif
namespace VEngine {

    extern Move killers[MAX_PLY][2]; 
    extern int history[2][64][64]; 
    extern Move counterMoves[12][64];
    extern Move plyMoves[MAX_PLY];
    extern int contHistory[64][64][64][64];
    extern int pawnCorrHist[2][16384];
    extern int nonPawnCorrHist[2][16384];
    extern int minorCorrHist[2][16384];
    extern int majorCorrHist[2][16384];



    int Search::see(Board& board, Move move) {
        int from = move.getFrom();
        int to = move.getTo();
        int piece = board.getPieceAt(from);
        int target = board.getPieceAt(to);
        
        const int seeValues[] = { 100, 305, 333, 563, 900, 20000, 0 };
        
        int gain[32]={0};
        int d = 0;
        
        gain[d] = seeValues[target];
        if (move.getFlags() >= FLAG_CAPTURE_MIN && target == None) gain[d] = seeValues[Pawn]; 

        uint64_t occ = board.occupancy[Both];
        Bitboard::popBit(occ, from);
        occ |= (1ULL << to);

        int stm = 1 - board.sideToMove; 
        int attackingPiece = piece;

        while (true) {
            d++;
            gain[d] = seeValues[attackingPiece] - gain[d - 1]; 
            if (std::max(-gain[d - 1], gain[d]) < 0) break;

            uint64_t attackers = board.getAttackers(to, occ) & board.occupancy[stm];
            if (!attackers) break;

            attackingPiece = -1;
            for (int pt = Pawn; pt <= King; pt++) {
                uint64_t subset = attackers & board.getPieces(pt, stm);
                if (subset) {
                    attackingPiece = pt;
                    int sq = Bitboard::getLsb(subset);
                    Bitboard::popBit(occ, sq); 
                    break;
                }
            }
            if (attackingPiece == -1) break;
            stm = 1 - stm;
        }

        while (--d > 0) {
            gain[d - 1] = -std::max(-gain[d - 1], gain[d]);
        }

        return gain[0];
    }



    void Search::orderMoves(Board& board, MoveList& moves, Stack* ss, int ply) {
        struct ScoredMove { Move m; int score; };
        ScoredMove scoredMoves[256]; 
        int count = moves.size();
        
        TTEntry ttEntry;
        Move hashMove;
        if (g_tt.probe(board.zorbitKey, ttEntry)) hashMove = ttEntry.move;

        const int pieceValues[] = {100, 305, 333, 563, 900, 20000, 0};
        
        const int BAD_CAPTURE_SCORE = 600000; 

        for (int i = 0; i < count; i++) {
            Move m = moves.moves[i];
            scoredMoves[i].m = m;
            int score = 0;

            if (m == hashMove) {
                score = HASH_MOVE_SCORE;
            } 
            else if (m.getFlags() >= FLAG_CAPTURE_MIN || m.isPromotion()) { 
                int victim = board.getPieceAt(m.getTo());
                int attacker = board.getPieceAt(m.getFrom());
                
                int vVal = (victim != None) ? pieceValues[victim] : pieceValues[Pawn]; 
                int aVal = (attacker != None) ? pieceValues[attacker] : 0;

                if (m.isPromotion()) {
                    score = PROMOTION_BONUS + pieceValues[(m.getFlags() & 3) + 1] + vVal; 
                } else {
                    if (vVal < aVal && see(board, m) < 0) {
                        score = BAD_CAPTURE_SCORE + (vVal * VICTIM_VALUE_MULTIPLIER) - aVal;
                    } else {
                        score = BASE_CAPTURE_SCORE + (vVal * VICTIM_VALUE_MULTIPLIER) - aVal;
                    }
                }
            } 
            else {
                if (killers[ply][0] == m) score = KILLER_1_SCORE;
                else if (killers[ply][1] == m) score = KILLER_2_SCORE;
                else {
                    score = history[board.sideToMove][m.getFrom()][m.getTo()];
                    Move prevM = (ss - 1)->currentMove;
                    if (prevM.getData() != 0) {
                        score += contHistory[prevM.getFrom()][prevM.getTo()][m.getFrom()][m.getTo()];
                        if (ply > 0 && plyMoves[ply - 1].getData() != 0) {
                            int prevPiece = board.getPieceAt(plyMoves[ply - 1].getTo());
                            if (prevPiece != None && counterMoves[prevPiece][plyMoves[ply - 1].getTo()] == m) {
                                score += COUNTER_MOVE_SCORE; 
                            }
                        }
                    }
                }
            }
            scoredMoves[i].score = score;
        }

        for (int i = 0; i < count - 1; i++) {
            int best_idx = i;
            for (int j = i + 1; j < count; j++) {
                if (scoredMoves[j].score > scoredMoves[best_idx].score) {
                    best_idx = j;
                }
            }
            if (best_idx != i) {
                ScoredMove temp = scoredMoves[i];
                scoredMoves[i] = scoredMoves[best_idx];
                scoredMoves[best_idx] = temp;
            }
        }

        for (int i = 0; i < count; i++) {
            moves.moves[i] = scoredMoves[i].m;
        }
    }
}