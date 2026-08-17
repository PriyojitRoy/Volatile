#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include "search/hce/TimeManager.hpp"
#include <iostream>

namespace VEngine {
    namespace Commands {
        void executeGo(UCIHandler& handler, std::istringstream& ss) {
            int wtime = 0, btime = 0, winc = 0, binc = 0;
            int movesToGo = 0, movetime = 0;
            bool infinite = false;
            bool ponder = false;

            std::string token;
            while (ss >> token) {
                if (token == "wtime") ss >> wtime;
                else if (token == "btime") ss >> btime;
                else if (token == "winc") ss >> winc;
                else if (token == "binc") ss >> binc;
                else if (token == "movestogo") ss >> movesToGo;
                else if (token == "movetime") ss >> movetime;
                else if (token == "infinite") infinite = true;
                else if (token == "ponder") ponder = true;
            }

            int optTime = 1000, maxTime = 1000; 

            if (movetime > 0) {
                optTime = maxTime = movetime; 
            } else if (wtime > 0 || btime > 0) {
                int currentPly = (handler.board.fullMoveNumber - 1) * 2 + (handler.board.sideToMove == Black ? 1 : 0);
                
                TimeAllocation alloc = TimeManager::calculateTime(wtime, btime, winc, binc, handler.board.sideToMove, currentPly, movesToGo);
                optTime = alloc.optimumMs;
                maxTime = alloc.maximumMs;
            } else if (infinite) {
                optTime = maxTime = 2147483647; 
            }

            handler.searcher.isPondering = ponder;

            if (handler.searchThread.joinable()) handler.searchThread.join();

            handler.searchThread = std::thread([&handler, optTime, maxTime]() {
                Move bestMove = handler.searcher.getBestMove(handler.board, optTime, maxTime);
                std::cout << "bestmove " << bestMove.toString() << std::endl;
            });
        }
    }
}
