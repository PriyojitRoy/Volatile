#include "search/TT.hpp"

namespace VEngine {

    TranspositionTable::TranspositionTable(unsigned long mb) {
        numEntries = (mb * 1024ULL * 1024ULL) / sizeof(TTEntry);
        table = new TTEntry[numEntries];
        clear();
    }

    TranspositionTable::~TranspositionTable() {
        delete[] table;
    }

    void TranspositionTable::clear() {
        if (table) {
           for (size_t i = 0; i < numEntries; i++) {
               table[i] = TTEntry();
            }
        }
    }
    void TranspositionTable::resize(unsigned long mb) {
        if (table != nullptr) {
            delete[] table;
        }
        numEntries = (mb * 1024ULL * 1024ULL) / sizeof(TTEntry);
        table = new TTEntry[numEntries];
        clear();
    }

    bool TranspositionTable::probe(uint64_t key, TTEntry& entry) {
        TTEntry* slot = &table[key % numEntries];
        if (slot->key == key) {
            entry = *slot;
            return true;
        }
        return false;
    }

    void TranspositionTable::store(uint64_t key, Move move, float value, int depth, TTNodeFlag flag, uint16_t age) {
        TTEntry* slot = &table[key % numEntries];
        
        if (slot->key != key || depth >= slot->depth || flag == TT_EXACT) {
            slot->key = key;
            if (move.getData() != 0) slot->move = move; 
            slot->value = value;
            slot->depth = depth;
            slot->flag = flag;
            slot->age = age;
        }
    }
}