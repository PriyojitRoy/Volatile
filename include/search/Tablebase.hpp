#ifndef CHESS_SEARCH_TABLEBASE_H
#define CHESS_SEARCH_TABLEBASE_H
#include "core/Board.hpp"
#include "core/Move.hpp"
#include <string>

namespace VEngine {
    class Tablebase {
    private:
        static bool loaded;
    public:
        static int MaxPieces;
        static void init(const std::string& path);
        static bool isLoaded();
        static bool probeWDL(const Board& board, int& wdlScore);
        static Move probeRoot(const Board& board, int& score);
    };
}

#endif // CHESS_SEARCH_TABLEBASE_H