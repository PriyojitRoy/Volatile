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



    int Search::quiescence(Board& board, int alpha, int beta, int ply, Stack* ss) {
        nodesSearched++; 
        if (!isPondering && ((nodesSearched & NODE_POLLING_MASK) == 0)) {
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() >= allocatedTime) {
                stopSearch = true;
            }
        }
        if (stopSearch) return 0;

        bool inCheck = board.isCheck();
        
        int standPat = -INF;
        int bestScore = -INF;

        if (!inCheck) {
            int rawEval = EvalBackend::evaluate(board);
            
            int pawnK, nonPawnK, minorK, majorK;
            getCorrHistKeys(board, pawnK, nonPawnK, minorK, majorK);
            int stm = board.sideToMove;
            int correction = CORR_WEIGHT_PAWN * pawnCorrHist[stm][pawnK] + 
                             CORR_WEIGHT_NONPAWN * nonPawnCorrHist[stm][nonPawnK] + 
                             CORR_WEIGHT_MINOR * minorCorrHist[stm][minorK] + 
                             CORR_WEIGHT_MAJOR * majorCorrHist[stm][majorK];

            standPat = std::clamp(rawEval + correction / CORR_EVAL_DIVISOR, -SCORE_MATE_BOUND, SCORE_MATE_BOUND);
            bestScore = standPat;

            if (standPat >= beta) return beta;
            if (alpha < standPat) alpha = standPat;
    
            if (standPat < alpha - QS_BIG_DELTA) return alpha; 
        }

        MoveList moves;
        if (inCheck) {
            MoveGen::generateAllMoves(board, moves);
        } else {
            MoveGen::generateCaptures(board, moves);
        }
        orderMoves(board, moves, ss, 0);

        int legalMoves = 0;
        for (int i = 0; i < moves.size(); i++) {
            Move m = moves.moves[i]; 
            if (m.getData() == ss->excludedMove.getData()) continue;
            if (!inCheck && m.getFlags() < FLAG_CAPTURE_MIN) continue; 
            if (!inCheck && m.getFlags() >= FLAG_CAPTURE_MIN && see(board, m) < QS_SEE_THRESHOLD) continue;

            if (!board.makeMove(m)) continue;
            legalMoves++;
            ss->currentMove = m;

            int score = -quiescence(board, -beta, -alpha, ply + 1, ss + 1);
            board.unmakeMove(m);

            if (score > bestScore) {
                bestScore = score;
                if (score >= beta) return beta;
                if (score > alpha) alpha = score;
            }
        }
        
        if (inCheck && legalMoves == 0) return -MATE_SCORE + ply;
        
        return bestScore;
    }
}