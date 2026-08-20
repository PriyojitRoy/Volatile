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
#include "search/Search.hpp"

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
        Search searcher;
    private:
        std::unordered_map<std::string, Command> commands;
        void registerCommands();
    };
}

#endif // VENGINE_UCI_UCIHANDLER_H