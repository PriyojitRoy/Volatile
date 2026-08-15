#ifndef CHESS_UCI_UCIHANDLER_H
#define CHESS_UCI_UCIHANDLER_H

#include <thread>
#include <string>
#include <vector>
#include "core/Board.hpp"
#include "core/Move.hpp"

// Include the appropriate search backend
#ifdef USE_NNUE
#include "search/nnue/MCTS.hpp"
#else
#include "search/hce/Search.hpp"
#endif

namespace VEngine {

    class UCIHandler {
    public:
        UCIHandler();
        void loop();

    private:
        Board board;
        std::thread searchThread;

        // The searcher type depends on the evaluation backend selected at build time
#ifdef USE_NNUE
        MCTS searcher;
#else
        Search searcher;
#endif

        uint64_t runPerft(Board& b, int depth);
        void playHuman();
        Move parseMove(const std::string& moveStr);
        void parsePosition(const std::string& input);
        void parseGo(const std::string& input);
        void playSelf(int movesToPlay);
        std::vector<std::string> split(const std::string& s, char delimiter);
    };
}

#endif // CHESS_UCI_UCIHANDLER_H