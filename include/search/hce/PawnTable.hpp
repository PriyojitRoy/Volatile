#ifndef CHESS_SEARCH_PAWNTABLE_H
#define CHESS_SEARCH_PAWNTABLE_H
#include <cstdint>
#include <vector>

namespace VEngine {

    struct PawnEntry {
        uint64_t key;     
        int score;        
    };

    class PawnTable {
    private:
        std::vector<PawnEntry> table;
        uint64_t mask;

    public:
        PawnTable(size_t mbSize = 2) {
            size_t numEntries = (mbSize * 1024 * 1024) / sizeof(PawnEntry);
            
            size_t powerOf2 = 1;
            while (powerOf2 <= numEntries) powerOf2 *= 2;
            powerOf2 /= 2;

            table.resize(powerOf2, {0, 0});
            mask = powerOf2 - 1;
        }

        void clear() {
            std::fill(table.begin(), table.end(), PawnEntry{0, 0});
        }

        bool probe(uint64_t key, int& outScore) const {
            const PawnEntry& entry = table[key & mask];
            if (entry.key == key) {
                outScore = entry.score;
                return true;
            }
            return false;
        }

        void store(uint64_t key, int score) {
            PawnEntry& entry = table[key & mask];
            entry.key = key;
            entry.score = score;
        }
    };

    extern PawnTable g_pawnTable;
}

#endif // CHESS_SEARCH_PAWNTABLE_H