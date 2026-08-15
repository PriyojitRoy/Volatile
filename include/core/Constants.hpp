#ifndef CHESS_CONSTANTS_H
#define CHESS_CONSTANTS_H

#include "common.h"

namespace VEngine {
    extern const int32_t PSQT[6][64];
    
    enum Color {
        White, Black,  Both
    };
    enum PieceType { Pawn, Knight, Bishop, Rook, Queen, King, None };

    enum Square : int {
        SqA1, SqB1, SqC1, SqD1, SqE1, SqF1, SqG1, SqH1,
        SqA2, SqB2, SqC2, SqD2, SqE2, SqF2, SqG2, SqH2,
        SqA3, SqB3, SqC3, SqD3, SqE3, SqF3, SqG3, SqH3,
        SqA4, SqB4, SqC4, SqD4, SqE4, SqF4, SqG4, SqH4,
        SqA5, SqB5, SqC5, SqD5, SqE5, SqF5, SqG5, SqH5,
        SqA6, SqB6, SqC6, SqD6, SqE6, SqF6, SqG6, SqH6,
        SqA7, SqB7, SqC7, SqD7, SqE7, SqF7, SqG7, SqH7,
        SqA8, SqB8, SqC8, SqD8, SqE8, SqF8, SqG8, SqH8,
        SqNone
    };

    enum Direction {
        North = 8, South = -8, East = 1, West = -1,
        NorthEast = 9, NorthWest = 7, SouthEast = -7, SouthWest = -9
    };


    struct EvalState {
        int32_t mg[2];
        int32_t eg[2];
        int32_t phase;
    };

    struct GameState {
        int castlingRights;
        int enPassantSq;
        int halfMoveClock;
        uint64_t zorbitKey;
        int capturedPiece; 
        int movedPiece; 
        EvalState evalState;

        uint64_t pawnKey; 
        uint64_t minorKey;
        uint64_t majorKey;
    };

    constexpr int RookSlots = 4096;
    constexpr int BishopSlots = 512;
    constexpr int MaxPly = 64;
    constexpr int MaxGameMoves = 2048;

    constexpr int Infinity = 50000;
    constexpr int MateValue = 49000;
    constexpr int MateScore = 48000;

    const std::string StartFen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    constexpr int CastlingWhiteKingSide = 1;
    constexpr int CastlingWhiteQueenSide = 2;
    constexpr int CastlingBlackKingSide = 4;
    constexpr int CastlingBlackQueenSide = 8;

    constexpr int ValuePawn = 100;
    constexpr int ValueKnight = 320;
    constexpr int ValueBishop = 330;
    constexpr int ValueRook = 500;
    constexpr int ValueQueen = 900;
    constexpr int ValueKing = 20000;
}

#endif // CHESS_CONSTANTS_H