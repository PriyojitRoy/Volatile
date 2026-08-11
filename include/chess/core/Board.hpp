#ifndef CHESS_BOARD_H
#define CHESS_BOARD_H

#include "chess/common.h"
#include "chess/core/Bitboard.hpp"
#include "chess/core/Move.hpp"
#include "chess/core/Constants.hpp"
#include "chess/eval/nnue/Accumulator.hpp"

namespace ChessCore {


    class Board {
    public:
        
        uint64_t pieces[6]; 
        uint64_t occupancy[3]; 
        
        int sideToMove;
        int enPassantSq;
        int castlingRights;
        int halfMoveClock;
        int fullMoveNumber;
        uint64_t zorbitKey;
        uint64_t pawnKey;
        uint64_t minorKey;
        uint64_t majorKey;
        EvalState evalState;
        Accumulator accumulator;
        void initZobrist();
        void initKeys();
        void reset();
        
        std::vector<GameState> history;
        
        Board();
        
        void parseFen(const std::string& fen);
        bool makeMove(Move move);
        void unmakeMove(Move move);
        void makeNullMove();
        void unmakeNullMove();
        int getKingSquare(int color) const; 
        uint64_t getPieces(int pieceType, int color) const; 
        int getNonPawnMaterial(const Board& board, int color);
        bool isSquareAttacked(int sq, int attackerSide) const;
        bool verifyZobrist();
        bool verifyKeys();
        bool isCheck();
        bool isInsufficientMaterial() const;
        bool isThreefoldRepetition() const;
        uint64_t getAllPieces() const {
            return occupancy[Both];
        }
        
        int getPieceAt(int sq) const;
        uint64_t getAttackers(int sq, uint64_t occ) const {
            uint64_t attackers = 0;
            
            // We use Bitboard:: prefix to call the fast inline functions
            attackers |= Bitboard::getKnightAttacks(sq) & getPieces(Knight, Both);
            attackers |= Bitboard::getKingAttacks(sq) & getPieces(King, Both);
            attackers |= Bitboard::getBishopAttacks(sq, occ) & (getPieces(Bishop, Both) | getPieces(Queen, Both));
            attackers |= Bitboard::getRookAttacks(sq, occ) & (getPieces(Rook, Both) | getPieces(Queen, Both));

            attackers |= Bitboard::getPawnAttacks(sq, Black) & getPieces(Pawn, White);
            attackers |= Bitboard::getPawnAttacks(sq, White) & getPieces(Pawn, Black);

            return attackers & occ;
        }
        void print();
        
        private:
        void hashBoard();
        void updateOccupancy();
    };
}

#endif // CHESS_BOARD_H