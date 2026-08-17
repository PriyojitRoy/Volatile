#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include <iostream>

namespace VEngine {
    namespace Commands {
        void executePerft(UCIHandler& handler, std::istringstream& ss) {
            std::string token;
            ss >> token;
            if (token.empty()) {
                std::cout << "Error: perft requires a depth argument." << std::endl;
                return;
            }
            int depth = std::stoi(token);
            int nodes = handler.runPerft(handler.board, depth);
            std::cout << "Nodes: " << nodes << std::endl;
        }
    }
}
