#include "search/hce/Search.hpp"
#include "core/MoveGen.hpp"
#include "search/TT.hpp"
#include "core/Bitboard.hpp"
#include "search/OpeningBook.hpp"
#include "search/Tablebase.hpp"

// Evaluation backend: compile with -DUSE_NNUE for NNUE, default is HCE
#ifdef USE_NNUE
#include "eval/nnue/Network.hpp"
namespace VEngine { using EvalBackend = Network; }
#else
#include "eval/hce/Evaluate.hpp"
namespace VEngine { using EvalBackend = Evaluate; }
#endif

namespace VEngine {

    extern TranspositionTable g_tt;

    static double LMRTable[MAX_PLY][MAX_PLY];
    Move killers[MAX_PLY][2]; 
    int history[2][64][64]; 
    Move counterMoves[12][64];
    Move plyMoves[MAX_PLY];
    int contHistory[64][64][64][64];
    static int failHighCount[MAX_PLY+10];
    int pawnCorrHist[2][16384];
    int nonPawnCorrHist[2][16384];
    int minorCorrHist[2][16384];
    int majorCorrHist[2][16384];


    static void updateHistory(int& currentScore, int bonus) {
        currentScore += bonus - currentScore * std::abs(bonus) / HISTORY_MAX_BONUS;
    }

    // void updateLMRTable(double c1, double c2) {
    //     for (int depth = 1; depth < MAX_PLY; depth++) {
    //         for (int moves = 1; moves < MAX_PLY; moves++) {
    //             LMRTable[depth][moves] = c1 + std::log(depth) * std::log(moves) / c2;
    //         }
    //     }
    // }

    static bool initLMR() {
        for (int depth = 1; depth < MAX_PLY; depth++) {
            for (int moves = 1; moves < MAX_PLY; moves++) {
                LMRTable[depth][moves] = c1 + std::log(depth) * std::log(moves) / c2;
            }
        }
        return true;
    }
    static bool _dummyLMR = initLMR();

    Search::Search() : stopSearch(false), isPondering(false), nodesSearched(0), allocatedTime(0) {
        std::memset(killers, 0, sizeof(killers));
        std::memset(history, 0, sizeof(history));
        std::memset(contHistory, 0, sizeof(contHistory));
        std::memset(pawnCorrHist, 0, sizeof(pawnCorrHist));
        std::memset(nonPawnCorrHist, 0, sizeof(nonPawnCorrHist));
        std::memset(minorCorrHist, 0, sizeof(minorCorrHist));
        std::memset(majorCorrHist, 0, sizeof(majorCorrHist));
    }
    Search::~Search() {}


    Move Search::getBestMove(Board& board, int optimumTime, int maximumTime) {
        
        Move bookMove = OpeningBook::getBookMove(board);
        if (bookMove.getData() != 0) return bookMove; 
        
        int totalPieces = Bitboard::countBits(board.occupancy[Both]);
        if (Tablebase::isLoaded() && totalPieces <= Tablebase::MaxPieces) {
            int tbScore = 0;
            Move tbMove = Tablebase::probeRoot(board, tbScore);
            if (tbMove.getData() != 0) {
                return tbMove;
            }
        }
        
        MoveList initialMoves;
        MoveGen::generateAllMoves(board, initialMoves);
        
        Stack stack[MAX_PLY + 10] = {};
        Stack* ss = stack + 2;
        
        orderMoves(board, initialMoves, ss, 0);
        int legalCount = 0;
        Move fallbackMove; 
        
        for (int i = 0; i < initialMoves.size(); i++) {
            Move m = initialMoves.moves[i];
            if (board.makeMove(m)) {
                legalCount++;
                if (legalCount == 1) fallbackMove = m; 
                board.unmakeMove(m);
            }
        }
        
        if (legalCount == 1) return fallbackMove; 

        startTime = std::chrono::high_resolution_clock::now();
        allocatedTime = std::max(MIN_ALLOC_TIME, maximumTime - TIME_MARGIN_MS); 
        stopSearch = false;
        nodesSearched = 0;

        std::memset(killers, 0, sizeof(killers));
        
        for (int c = 0; c < 2; c++) {
            for (int from = 0; from < 64; from++) {
                for (int to = 0; to < 64; to++) {
                    history[c][from][to] /= HISTORY_AGE_DIVISOR; 
                }
            }
        }
        
        Move bestMove = legalCount > 0 ? fallbackMove : Move(); 
        Move prevBestMove;
        int prevScore = 0;
        int stabilityCounter = 0;
        
        double moveInstability = 0.0;
        double scoreVolatility = 0.0;
        
        int alpha = -INF;
        int beta = INF;

        for (int depth = 1; depth <= MAX_PLY; depth++) {
            int score = 0;

            if (depth >= ASPIRATION_MIN_DEPTH) {
                int delta = ASPIRATION_INIT_DELTA;
                alpha = std::max(-INF, prevScore - delta);
                beta = std::min(INF, prevScore + delta);
                int failCount = 0;

                while (true) {
                    score = negamax(board, depth, alpha, beta, 0, ss, true);
                    if (stopSearch) break;

                    if (score <= alpha) {
                        failCount++;
                        alpha = std::max(-INF, alpha - delta);
                    } 
                    else if (score >= beta) {
                        failCount++;
                        beta = std::min(INF, beta + delta);
                    } 
                    else {
                        break; 
                    }
                    
                    delta += delta / 2;
                    
                    if (failCount >= ASPIRATION_MAX_FAILS || delta > ASPIRATION_MAX_DELTA) {
                        alpha = -INF;
                        beta = INF;
                    }
                }
            } else {
                alpha = -INF;
                beta = INF;
                score = negamax(board, depth, alpha, beta, 0, ss, true);
            }
            
            if (stopSearch) { 
                if (depth > 1) {
                    bestMove = prevBestMove;
                    score = prevScore; 
                }
                break;
            }

            TTEntry entry;
            if (g_tt.probe(board.zorbitKey, entry)) {
                if (entry.move.getData() != 0) {
                    for (int j = 0; j < initialMoves.size(); j++) {
                        Move validMove = initialMoves.moves[j];
                        if (validMove.getData() == entry.move.getData()) {
                            bestMove = entry.move;
                            break;
                        }
                    }
                }
            }

            auto now = std::chrono::high_resolution_clock::now();
            int elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count();

            if (depth > 1) {
                if (bestMove.getData() != prevBestMove.getData()) {
                    moveInstability += 1.0; 
                    stabilityCounter = 0; 
                } else {
                    stabilityCounter++;
                }
                
                moveInstability *= 0.6; 
                
                int scoreDiff = std::abs(score - prevScore);
                scoreVolatility = (scoreVolatility * 0.5) + (scoreDiff / 50.0);
            }

            std::cout << "info depth " << depth;
            
            if (std::abs(score) > SCORE_MATE_BOUND) {
                int mateIn = (MATE_SCORE - std::abs(score) + 1) / 2;
                std::cout << " score mate " << (score > 0 ? mateIn : -mateIn);
            } else {
                std::cout << " score cp " << score;
            }
            
            std::cout << " time " << elapsed 
              << " nodes " << nodesSearched 
              << " nps " << (elapsed > 0 ? (nodesSearched * 1000LL) / elapsed : 0)
              << " pv " << bestMove.toString() 
              << std::endl;

            prevBestMove = bestMove;
            prevScore = score;
            if (std::abs(score) > SCORE_MATE_BOUND) break; 

            double panicFactor = 1.0 + moveInstability + scoreVolatility;
            int currentOptimum = std::min((int)(optimumTime * panicFactor), maximumTime);

            if (optimumTime != maximumTime && depth >= TIME_STABILITY_DEPTH && stabilityCounter >= TIME_STABILITY_MIN) {
               if (elapsed >= currentOptimum * OPTIMUM_TIME_RATIO) break;               
            }

            if (!isPondering && elapsed >= currentOptimum) {
                break; 
            }
        }
        
        lastEvalScore = prevScore;
        return bestMove;
    }
    void getCorrHistKeys(const Board& board, int& pawnK, int& nonPawnK, int& minorK, int& majorK) {
        pawnK = board.pawnKey & 16383;
        minorK = board.minorKey & 16383;
        majorK = board.majorKey & 16383;
        
        nonPawnK = (board.minorKey ^ board.majorKey) & 16383;
    }
    int Search::negamax(Board& board, int depth, int alpha, int beta, int ply, Stack* ss, bool isPV) {

        if (ply >= MAX_PLY - 1) return EvalBackend::evaluate(board);

        int originalAlpha = alpha;

        nodesSearched++;
        
        if (maxNodesLimit > 0 && (uint64_t)nodesSearched >= maxNodesLimit) {
            stopSearch = true;
            return alpha;
        }
        if (!isPondering && ((nodesSearched & NODE_POLLING_MASK) == 0)) {
            auto now = std::chrono::high_resolution_clock::now();
            if (std::chrono::duration_cast<std::chrono::milliseconds>(now - startTime).count() >= allocatedTime)
                stopSearch = true;
        }
        if (stopSearch) return 0;

        if (ply > 0 && (board.isThreefoldRepetition() || board.halfMoveClock >= 100)) return 0;

        int mateValue = MATE_SCORE - ply;
        if (alpha < -mateValue) alpha = -mateValue;
        if (beta > mateValue - 1) beta = mateValue - 1;
        if (alpha >= beta) return alpha;

        if (ply < MAX_PLY) failHighCount[ply + 1] = 0;

        int totalPieces = Bitboard::countBits(board.occupancy[Both]);
        if (Tablebase::isLoaded() && totalPieces <= Tablebase::MaxPieces) {
            int tbScore;
            if (Tablebase::probeWDL(board, tbScore)) {
                if (tbScore > 0) return TB_WIN_SCORE - ply;
                if (tbScore < 0) return -TB_WIN_SCORE + ply;
                return 0; 
            }
        }
        
        TTEntry ttEntry;
        Move hashMove;
        bool isZugzwangRisk = (totalPieces <= 6);
        bool haveHashMove = g_tt.probe(board.zorbitKey, ttEntry);
        int ttValue = ttEntry.value;

        if (haveHashMove) {
            hashMove = ttEntry.move;
            if (ttValue > SCORE_MATE_BOUND) ttValue -= ply;
            else if (ttValue < -SCORE_MATE_BOUND) ttValue += ply;

            if (ttEntry.depth >= depth && ply > 0 && !isPV && ss->excludedMove.getData() == 0) {
                if (ttEntry.flag == TT_EXACT) return ttValue;
                if (ttEntry.flag == TT_LOWERBOUND) alpha = std::max(alpha, ttValue);
                if (ttEntry.flag == TT_UPPERBOUND) beta = std::min(beta, ttValue);
                if (alpha >= beta) return ttValue;
            }
        }

        if (depth >= IID_MIN_DEPTH && hashMove.getData() == 0 && !isZugzwangRisk) {
            int iidDepth = isPV ? (depth - 2) : (depth / 2);
            negamax(board, iidDepth, alpha, beta, ply, ss, isPV);
            
            haveHashMove = g_tt.probe(board.zorbitKey, ttEntry);
            if (haveHashMove) {
                hashMove = ttEntry.move;
                ttValue = ttEntry.value;
                if (ttValue > SCORE_MATE_BOUND) ttValue -= ply;
                else if (ttValue < -SCORE_MATE_BOUND) ttValue += ply;
            }
        }

        if (depth <= 0) return quiescence(board, alpha, beta, ply, ss);

        int pawnK, nonPawnK, minorK, majorK;
        getCorrHistKeys(board, pawnK, nonPawnK, minorK, majorK);
        int stm = board.sideToMove;
        int correction = CORR_WEIGHT_PAWN * pawnCorrHist[stm][pawnK] + 
                         CORR_WEIGHT_NONPAWN * nonPawnCorrHist[stm][nonPawnK] + 
                         CORR_WEIGHT_MINOR * minorCorrHist[stm][minorK] + 
                         CORR_WEIGHT_MAJOR * majorCorrHist[stm][majorK];

        int lazyRaw = EvalBackend::lazyEvaluate(board);
        int lazyScore = std::clamp(lazyRaw + correction / CORR_EVAL_DIVISOR, -SCORE_MATE_BOUND, SCORE_MATE_BOUND);
        
        int staticEval;

        if (lazyScore + LAZY_MARGIN <= alpha || lazyScore - LAZY_MARGIN >= beta) {
            staticEval = lazyScore;
        } else {
            int rawEval = EvalBackend::evaluate(board);
            staticEval = std::clamp(rawEval + correction / CORR_EVAL_DIVISOR, -SCORE_MATE_BOUND, SCORE_MATE_BOUND);
        }
        
        ss->staticEval = staticEval;
        
        bool inCheck = board.isCheck(); 
        
        bool improving = !inCheck && ply >= 2 && (ss->staticEval >= (ss - 2)->staticEval || (ss - 2)->staticEval == 0);
        
        int eval = staticEval;
        if (haveHashMove && !inCheck && std::abs(ttValue) < SCORE_MATE_BOUND) {
            if (ttEntry.flag != (ttValue > staticEval ? TT_UPPERBOUND : TT_LOWERBOUND)) {
                eval = ttValue;
            }
        }
        
        if (depth <= RAZOR_MIN_DEPTH && !isPV && !inCheck && !isZugzwangRisk && std::abs(alpha) < SCORE_MATE_BOUND) {
            int razorMargin = RAZOR_BASE_MARGIN + (RAZOR_DEPTH_MULTIPLIER * depth * depth); 
            if (eval + razorMargin <= alpha) {
                int qsScore = quiescence(board, alpha, beta, ply, ss);
                if (qsScore <= alpha) return qsScore;
            }
        }

        if (depth <= RFP_MIN_DEPTH && !isPV && !inCheck && !isZugzwangRisk && std::abs(beta) < SCORE_MATE_BOUND) {
            int rfpMargin = improving 
                ? (RFP_BASE_MARGIN / 2) + (RFP_DEPTH_MULTIPLIER * (depth - 1)) 
                : RFP_BASE_MARGIN + (RFP_DEPTH_MULTIPLIER * depth);
                
            rfpMargin += (depth * depth * 2);

            if (eval - rfpMargin >= beta) {
                return (eval + beta) / 2;
            }
        }

        if (depth >= NMP_MIN_DEPTH && !isPV && !inCheck && ply > 0 && !isZugzwangRisk && eval >= beta) {
            board.makeNullMove(); 
            ss->currentMove = Move();
            
            int R = 4 + depth / 4 + std::min(3, (eval - beta) / 200);
            if(!improving) R--;
            if (eval - beta > NMP_BETA_MARGIN) R++; 

            int nullScore = -negamax(board, depth - R - 1, -beta, -beta + 1, ply + 1, ss + 1 , false);
            board.unmakeNullMove();
            if (stopSearch) return 0;
            
            if (nullScore >= beta) {
                return (nullScore >= SCORE_MATE_BOUND) ? beta : nullScore; 
            }
        }
        
        if (depth >= PROBCUT_MIN_DEPTH && !isPV && !inCheck && !isZugzwangRisk && std::abs(beta) < SCORE_MATE_BOUND) {
            int probBeta = beta + PROBCUT_BETA_MARGIN;
            int probDepth = depth - 4;
            int probScore = -negamax(board, probDepth, -probBeta, -probBeta + 1, ply + 1, ss + 1, false);
            if (stopSearch) return 0;

            if (probScore >= probBeta) {
                return probScore; 
            }
        }
        
        bool isSingular = false;
        int seExtension = 0;

        bool do_singular_search = !isPV && depth >= SE_MIN_DEPTH 
                                  && haveHashMove && hashMove.getData() != 0
                                  && ttEntry.depth >= depth - SE_TT_DEPTH_MARGIN 
                                  && ttEntry.flag == TT_LOWERBOUND 
                                  && ss->excludedMove.getData() == 0
                                  && std::abs(ttValue) < SCORE_MATE_BOUND;

        if (do_singular_search) {
            int singularBeta = std::max(-MATE_SCORE, ttValue - depth);
            int singularDepth = (depth - 1) / 2;      

            ss->excludedMove = hashMove; 
            int seScore = negamax(board, singularDepth, singularBeta - 1, singularBeta, ply, ss, false);
            ss->excludedMove = Move();   

            if (seScore < singularBeta) {
                isSingular = true;
                seExtension = 1;
                if (!isPV && seScore < singularBeta - SE_SCORE_MARGIN) {
                    seExtension = 2; 
                }
            } 
            else if (ttValue >= beta) {
                seExtension = -2;
            }
        }

        MoveList moves;
        MoveGen::generateAllMoves(board, moves);
        orderMoves(board, moves, ss, ply); 

        int legalMoves = 0;
        int bestScore = -INF;
        Move bestMove;
        Move quietsSearched[256];
        int quietCount = 0;

        for (int i = 0; i < moves.size(); i++) {
            Move m = moves.moves[i];
            if (m.getData() == ss->excludedMove.getData()) continue;
            Move prevM = (ss - 1)->currentMove;
            int contScore = (prevM.getData() != 0) ? contHistory[prevM.getFrom()][prevM.getTo()][m.getFrom()][m.getTo()] : 0;
            int histScore = history[board.sideToMove][m.getFrom()][m.getTo()] + contScore;
            bool isQuiet = (m.getFlags() < FLAG_CAPTURE_MIN);
            bool isHash = (m.getData() == hashMove.getData() && hashMove.getData() != 0);

            bool isBadCapture = (!isQuiet && !isHash && see(board, m) < 0);

            if (depth <= 4 && !inCheck && !isPV && isQuiet) {
                if (histScore < -2000 && !(killers[ply][0] == m) && !(killers[ply][1] == m)) {
                    continue;
                }
            }

            int lmpThreshold = (improving ? 3 : 2) + (depth * 2);
            if (depth <= LMP_MIN_DEPTH && !inCheck && !isPV && isQuiet && legalMoves > lmpThreshold) {
                if (!(killers[ply][0] == m) && !(killers[ply][1] == m) && histScore < GREAT_HISTORY_SCORE) {
                    continue; 
                }
            }

            if (depth <= FUTILITY_MIN_DEPTH && !inCheck && !isPV && isQuiet && std::abs(alpha) < SCORE_MATE_BOUND) {
                bool isKiller = (killers[ply][0] == m || killers[ply][1] == m);
                bool isAdvancedPawn = (board.getPieceAt(m.getFrom()) == Pawn && (m.getTo() >= 48 || m.getTo() <= 15));
                bool hasGreatHistory = (histScore > GREAT_HISTORY_SCORE); 

                if (!isHash && !isKiller && !isAdvancedPawn && !hasGreatHistory) {
                    int futilityMargin = FUTILITY_BASE_MARGIN + depth * FUTILITY_DEPTH_MARGIN; 
                    if (eval + futilityMargin <= alpha) {
                        continue; 
                    }
                }
            }

            if (depth <= SEE_PRUNING_MAX_DEPTH && !inCheck && !isPV && !isQuiet) {
                int seeThreshold = -depth * SEE_PRUNING_MULTIPLIER;
                if (!isHash && see(board, m) < seeThreshold) continue; 
            }

            if (!board.makeMove(m)) continue;
            legalMoves++;
            
            ss->currentMove = m;
            plyMoves[ply] = m;
            
            if (isQuiet) {
                quietsSearched[quietCount++] = m;
            }

            int extension = inCheck ? 1 : 0;
            if (isSingular && m.getData() == hashMove.getData()) extension += seExtension;
            int score;

            if (legalMoves == 1) {
                score = -negamax(board, depth - 1 + extension, -beta, -alpha, ply + 1, ss + 1, true);
            } else {
                int reduction = 0;
                
                if (depth >= LMR_MIN_DEPTH_REDUCT && !inCheck && legalMoves > LMR_MIN_LEGAL_MOVES && (isQuiet || isBadCapture)) {
                    int movesToUse = std::min(legalMoves, LMR_MAX_MOVES_CALC);
                    reduction = (int)LMRTable[depth][movesToUse];
                    
                    if (isQuiet) {
                        if (histScore > GREAT_HISTORY_SCORE) reduction--;
                        else if (histScore < 0) reduction++;
                    }

                    if (isPV) reduction--;
                    if (!improving) reduction++;
                    
                    if (failHighCount[ply + 1] > 2) reduction++;
                    if (killers[ply][0] == m || killers[ply][1] == m) reduction--;
                    
                    if (isBadCapture) reduction++; 

                    reduction = std::clamp(reduction, 0, depth - 2);
                }

                score = -negamax(board, depth - 1 - reduction + extension, -alpha - 1, -alpha, ply + 1, ss + 1, false);
                if (score > alpha && reduction > 0) {
                     score = -negamax(board, depth - 1 + extension, -alpha - 1, -alpha, ply + 1, ss+1, false);
                }
                if (score > alpha && score < beta) {
                     score = -negamax(board, depth - 1 + extension, -beta, -alpha, ply + 1, ss+1,true);
                }
            }

            board.unmakeMove(m);
            if (stopSearch) return 0;

            if (score > bestScore) {
                bestScore = score;
                bestMove = m;
                if (score > alpha) {
                    alpha = score;
                    if (alpha >= beta) { 
                        failHighCount[ply]++;
                        if (isQuiet) {
                            killers[ply][1] = killers[ply][0];
                            killers[ply][0] = m;
                            
                            int bonus = depth * depth;
                            updateHistory(history[board.sideToMove][m.getFrom()][m.getTo()], bonus);
                            
                            Move prevM2 = (ss - 1)->currentMove;
                            if (prevM2.getData() != 0) {
                                updateHistory(contHistory[prevM2.getFrom()][prevM2.getTo()][m.getFrom()][m.getTo()], bonus);
                            }
                            
                            for (int idx = 0; idx < quietCount; idx++) {
                                Move quietMove = quietsSearched[idx];
                                if (quietMove.getData() != m.getData()) {
                                    updateHistory(history[board.sideToMove][quietMove.getFrom()][quietMove.getTo()], -bonus);
                                    
                                    if (prevM2.getData() != 0) {
                                        updateHistory(contHistory[prevM2.getFrom()][prevM2.getTo()][quietMove.getFrom()][quietMove.getTo()], -bonus);
                                    }
                                }
                            }
                            if (ply > 0 && plyMoves[ply - 1].getData() != 0) {
                                int prevPiece = board.getPieceAt(plyMoves[ply - 1].getTo());
                                if (prevPiece != None && counterMoves[prevPiece][plyMoves[ply - 1].getTo()] == m) {
                                    score += COUNTER_MOVE_SCORE; 
                                }
                            }
                        }
                        break; 
                    }
                }
            }
        }

        if (legalMoves == 0) return inCheck ? (-MATE_SCORE + ply) : 0;

        TTNodeFlag flag = (bestScore >= beta) ? TT_LOWERBOUND : (bestScore > originalAlpha ? TT_EXACT : TT_UPPERBOUND);

        if (!inCheck && legalMoves > 0 && ss->excludedMove.getData() == 0 && bestMove.getFlags() < FLAG_CAPTURE_MIN) {
            if (!(flag == TT_LOWERBOUND && bestScore <= staticEval) &&
                !(flag == TT_UPPERBOUND && bestScore >= staticEval)) {
                
                int diff = std::clamp(bestScore - staticEval, -CORR_CLAMP_LIMIT, CORR_CLAMP_LIMIT);
                int stm2 = board.sideToMove;

                pawnCorrHist[stm2][pawnK] += diff - pawnCorrHist[stm2][pawnK] * std::abs(diff) / CORR_HISTORY_DIVISOR;
                nonPawnCorrHist[stm2][nonPawnK] += diff - nonPawnCorrHist[stm2][nonPawnK] * std::abs(diff) / CORR_HISTORY_DIVISOR;
                minorCorrHist[stm2][minorK] += diff - minorCorrHist[stm2][minorK] * std::abs(diff) / CORR_HISTORY_DIVISOR;
                majorCorrHist[stm2][majorK] += diff - majorCorrHist[stm2][majorK] * std::abs(diff) / CORR_HISTORY_DIVISOR;
            }
        }

        int ttStoreValue = bestScore;
        if (ttStoreValue > SCORE_MATE_BOUND) ttStoreValue += ply;
        else if (ttStoreValue < -SCORE_MATE_BOUND) ttStoreValue -= ply;

        g_tt.store(board.zorbitKey, bestMove, (float)ttStoreValue, depth, flag, 0);

        return bestScore;
    }
}
