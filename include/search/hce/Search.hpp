#ifndef CHESS_SEARCH_SEARCH_H
#define CHESS_SEARCH_SEARCH_H

#include "common.h"
#include "core/Board.hpp"
#include "core/Move.hpp"
#include "core/MoveGen.hpp"
#include "search/TT.hpp"

namespace VEngine {

    // Basic Search Constants
    const int INF                       =50000;
    const int MATE_SCORE                =30000;
    const int MAX_PLY                   =256;
    const int SCORE_MATE_BOUND         =29000;
    const int TB_WIN_SCORE             =20000;
    const int NODE_POLLING_MASK        =2047;
    const int TIME_MARGIN_MS           =15;
    const int MIN_ALLOC_TIME           =10;
   
   // Time Management
    const double OPTIMUM_TIME_RATIO       =0.60;
    const int TIME_STABILITY_DEPTH     =7;
    const int TIME_STABILITY_MIN       =4;
   
   // Aspiration Windows
    const int ASPIRATION_MIN_DEPTH     =5;
    const int ASPIRATION_INIT_DELTA    =50;
    const int ASPIRATION_MAX_FAILS     =3;
    const int ASPIRATION_MAX_DELTA     =1000;
   
   // Pruning Margins & Depth Limits
    const int IID_MIN_DEPTH            =5;
    const int RAZOR_MIN_DEPTH          =3;
    const int RAZOR_BASE_MARGIN        =411;
    const int RAZOR_DEPTH_MULTIPLIER   =40;
    const int RFP_MIN_DEPTH            =8;
    const int RFP_BASE_MARGIN          =144;
    const int RFP_DEPTH_MULTIPLIER     =30;
    const int NMP_MIN_DEPTH            =3;
    const int NMP_BETA_MARGIN          =210;
    const int PROBCUT_MIN_DEPTH        =5;
    const int PROBCUT_BETA_MARGIN      =200;
    const int SE_MIN_DEPTH             =6;
    const int SE_TT_DEPTH_MARGIN       =3;
    const int SE_SCORE_MARGIN          =16;
    const int LMP_MIN_DEPTH            =3;
    const int LMP_MIN_HISTORY          =2000;
    const int FUTILITY_MIN_DEPTH       =4;
    const int FUTILITY_BASE_MARGIN     =89;
    const int FUTILITY_DEPTH_MARGIN    =100;
    const int QS_SEE_THRESHOLD         =-105;
    const int QS_BIG_DELTA             =2000;
    const int SEE_PRUNING_MAX_DEPTH    =4;
    const int SEE_PRUNING_MULTIPLIER   =350;
    
    // Move Ordering Scores
    const int HASH_MOVE_SCORE          =2000000;
    const int BASE_CAPTURE_SCORE       =1000000;
    const int PROMOTION_BONUS          =900000;
    const int KILLER_1_SCORE           =900000;
    const int KILLER_2_SCORE           =800000;
    const int COUNTER_MOVE_SCORE       =700000;
    const int GREAT_HISTORY_SCORE      =4000;
    const int VICTIM_VALUE_MULTIPLIER  =10;
    const int LAZY_MARGIN              =300;
    
    // History & Reductions
    const int HISTORY_MAX_BONUS        =16384;
    const int HISTORY_AGE_DIVISOR      =8;
    const int LMR_MIN_DEPTH_REDUCT     =2;
    const int LMR_MIN_LEGAL_MOVES      =4;
    const int LMR_MAX_MOVES_CALC       =63;
    const int LMR_HISTORY_DIVISOR      =7921;
    
    // Evaluation Corrections
    const int CORR_WEIGHT_PAWN         =202;
    const int CORR_WEIGHT_NONPAWN      =141;
    const int CORR_WEIGHT_MINOR        =180;
    const int CORR_WEIGHT_MAJOR        =151;
    const int CORR_EVAL_DIVISOR        =2048;
    const int CORR_HISTORY_DIVISOR     =1024;
    const int CORR_CLAMP_LIMIT         =256;
    
    // Move Flags
    const int FLAG_CAPTURE_MIN         =4;

    //LMR Table constants
    const double c1                    =0.75;
    const double c2                    =2.25;

    

    struct Stack {
        Move currentMove;
        Move excludedMove;
        int staticEval = 0;
        int reduction = 0;
    };

void getCorrHistKeys(const Board& board, int& pawnK, int& nonPawnK, int& minorK, int& majorK);

    class Search {
    public:
        Search();
        ~Search();
        std::atomic<bool> stopSearch{false};
        std::atomic<bool> isPondering{false};
        Move getBestMove(Board& board, int optimumTime, int maximumTime);
        std::chrono::time_point<std::chrono::high_resolution_clock> startTime;
        int lastEvalScore = 0;
        uint64_t maxNodesLimit = 0;

    private:
        int negamax(Board& board, int depth, int alpha, int beta, int ply, Stack* ss, bool isPV);
        int quiescence(Board& board, int alpha, int beta, int ply, Stack* ss);
        int see(Board& board, Move move);
        void orderMoves(Board& board, MoveList& moves, Stack* ss, int ply = 0);

        int nodesSearched;
        int allocatedTime;
        Board rootBoard;
    };
}

#endif // CHESS_SEARCH_SEARCH_H
