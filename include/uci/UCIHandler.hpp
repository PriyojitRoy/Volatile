#ifndef VENGINE_UCI_UCIHANDLER_H
#define VENGINE_UCI_UCIHANDLER_H

#include <thread>
#include <string>
#include <vector>
#include <unordered_map>
#include <functional>
#include <sstream>
#include "core/Board.hpp"
#include "core/Move.hpp"

// Include the appropriate search backend
#ifdef USE_NNUE
#include "search/nnue/MCTS.hpp"
#else
#include "search/hce/Search.hpp"
#endif

namespace VEngine {

    struct Command {
        std::string syntax;
        std::string description;
        std::function<void(std::istringstream&)> handler;
    };

    class UCIHandler {
    public:
        UCIHandler();
        void loop();

        Move parseMove(const std::string& moveStr);
        uint64_t runPerft(Board& b, int depth);

        Board board;
        std::thread searchThread;
        bool isRunning;

#ifdef USE_NNUE
        MCTS searcher;
#else
        Search searcher;
#endif

    private:
        std::unordered_map<std::string, Command> commands;
        void registerCommands();
    };
}

#endif // VENGINE_UCI_UCIHANDLER_H