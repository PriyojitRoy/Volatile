#ifndef CHESS_SEARCH_TT_H
#define CHESS_SEARCH_TT_H
#include <cstdint>
#include "chess/core/Move.hpp"

namespace ChessCore {

    enum TTNodeFlag { TT_EXACT, TT_LOWERBOUND, TT_UPPERBOUND };

    struct TTEntry {
        uint64_t key;
        Move move;
        float value;
        int depth;
        TTNodeFlag flag;
        uint16_t age;
    };

    class TranspositionTable {
    public:
        TranspositionTable(unsigned long mb);
        ~TranspositionTable();
        inline void prefetch(uint64_t key) const {
            __builtin_prefetch(&table[key % numEntries]);
        }
        void clear();
        void resize(unsigned long mb);
        bool probe(uint64_t key, TTEntry& entry);
        void store(uint64_t key, Move move, float value, int depth, TTNodeFlag flag, uint16_t age);

    private:
        TTEntry* table;
        uint64_t numEntries;
    };

    extern TranspositionTable g_tt;
}

#endif // CHESS_SEARCH_TT_H