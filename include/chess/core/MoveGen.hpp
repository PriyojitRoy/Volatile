#ifndef CHESS_MOVEGEN_H
#define CHESS_MOVEGEN_H

#include "chess/common.h"
#include "chess/core/Board.hpp"
#include "chess/core/Move.hpp"

namespace ChessCore {

    struct MoveList {
        Move moves[256];
        int count;

        MoveList() : count(0) {}

        inline void add(const Move& move) {
            moves[count++] = move;
        }

        inline void reset() {
            count = 0;
        }
        
        inline int size() const {
            return count;
        }
        
        inline const Move& operator[](int index) const {
            return moves[index];
        }
    };

    class MoveGen {
    public:
    static void generateAllMoves(const Board& board, MoveList& moveList);
    static void generateCaptures(const Board& board, MoveList& moveList);

    private:
        static void generatePawnMoves(const Board& board, MoveList& moveList);
        static void addPromotions(int from, int to, MoveList& moves, bool capture);
        static void generateKnightMoves(const Board& board, MoveList& moveList);
        static void generateBishopMoves(const Board& board, MoveList& moveList);
        static void generateRookMoves(const Board& board, MoveList& moveList);
        static void generateQueenMoves(const Board& board, MoveList& moveList);
        static void generateKingMoves(const Board& board, MoveList& moveList);
        static void generateCastlingMoves(const Board& board, MoveList& moveList);
    };
}

#endif // CHESS_MOVEGEN_H