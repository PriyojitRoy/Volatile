#ifndef CHESS_SEARCH_OPENINGBOOK_H
#define CHESS_SEARCH_OPENINGBOOK_H
#include "chess/core/Board.hpp"
#include "chess/core/Move.hpp"
#include <string>
#include <vector>

namespace ChessCore {

    struct PolyglotEntry {
        uint64_t key;
        uint16_t move;
        uint16_t weight;
        uint32_t learn;
    };

    class OpeningBook {
    public:
        // Load a .bin file into memory
        static bool load(const std::vector<std::string>& filenames);        
        // Query the book for a move matching the current board
        static Move getBookMove(Board& board);
        
        static bool hasBook;

    private:
        static std::vector<PolyglotEntry> entries;
        
        static uint64_t computePolyglotHash(const Board& board);
        
        static const uint64_t PolyglotRandoms[781]; 
    };
}

#endif // CHESS_SEARCH_OPENINGBOOK_H