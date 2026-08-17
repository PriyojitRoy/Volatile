#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include <iostream>

namespace VEngine {
    namespace Commands {
        void executePosition(UCIHandler& handler, std::istringstream& ss) {
            handler.searcher.stopSearch = true;
            if (handler.searchThread.joinable()) handler.searchThread.join();

            std::string token, type;
            if (!(ss >> type)) return;

            if (type == "startpos") {
                handler.board.reset(); 
                handler.board.parseFen(StartFen);
            } else if (type == "fen") {
                std::string fen;
                while (ss >> token && token != "moves") {
                    fen += token + " ";
                }
                handler.board.reset();
                handler.board.parseFen(fen);
            }

            if (token == "moves" || (ss >> token && token == "moves")) {
                std::string moveStr;
                while (ss >> moveStr) {
                    Move m = handler.parseMove(moveStr);
                    if (m.getData() != 0) { 
                        handler.board.makeMove(m); 
                    }
                    else std::cout << "info string FATAL ERROR: Desync! Could not parse move: " << moveStr << std::endl;
                }
            }
        }
    }
}
