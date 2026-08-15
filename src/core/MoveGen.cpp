#include "core/MoveGen.hpp"
#include "core/Bitboard.hpp"
#include "core/Constants.hpp"

namespace VEngine {

    void MoveGen::generateAllMoves(const Board& board, MoveList& moveList) {
        moveList.reset();
        generatePawnMoves(board, moveList);
        generateKnightMoves(board, moveList);
        generateBishopMoves(board, moveList);
        generateRookMoves(board, moveList);
        generateQueenMoves(board, moveList);
        generateKingMoves(board, moveList);
        generateCastlingMoves(board, moveList);
    }

   void MoveGen::generatePawnMoves(const Board& board, MoveList& moveList) {
        int us = board.sideToMove;
        int them = 1 - us;
        
        uint64_t pawns = board.getPieces(Pawn, us);
        uint64_t enemies = board.occupancy[them];
        uint64_t empty = ~board.getAllPieces();
        
        int up = (us == White) ? 8 : -8;
        int promRankStart = (us == White) ? 56 : 0;
        int promRankEnd   = (us == White) ? 63 : 7;
        
        uint64_t singlePush = (us == White) ? (pawns << 8) : (pawns >> 8);
        singlePush &= empty;

        uint64_t pushes = singlePush;
        while (pushes) {
            int to = Bitboard::getLsb(pushes);
            int from = to - up;
            if (to >= promRankStart && to <= promRankEnd) {
                moveList.add(Move(from, to, PromotionQueen));
                moveList.add(Move(from, to, PromotionRook));
                moveList.add(Move(from, to, PromotionBishop));
                moveList.add(Move(from, to, PromotionKnight));
            } else {
                moveList.add(Move(from, to, Quiet));
            }
            Bitboard::popLsb(pushes);
        }

        // Check start rank specifically: Rank 2 for White (8-15), Rank 7 for Black (48-55)
        if (us == White) {
            uint64_t rank2Pawns = pawns & 0x000000000000FF00ULL;
            uint64_t doublePush = (rank2Pawns << 8) & empty; // First step
            doublePush = (doublePush << 8) & empty;         // Second step
            while (doublePush) {
                int to = Bitboard::getLsb(doublePush);
                moveList.add(Move(to - 16, to, DoublePawnPush));
                Bitboard::popLsb(doublePush);
            }
        } else {
            uint64_t rank7Pawns = pawns & 0x00FF000000000000ULL;
            uint64_t doublePush = (rank7Pawns >> 8) & empty;
            doublePush = (doublePush >> 8) & empty;
            while (doublePush) {
                int to = Bitboard::getLsb(doublePush);
                moveList.add(Move(to + 16, to, DoublePawnPush));
                Bitboard::popLsb(doublePush);
            }
        }

        //Check for En Passant
        const uint64_t notAFile = 0xFEFEFEFEFEFEFEFEULL;
        const uint64_t notHFile = 0x7F7F7F7F7F7F7F7FULL;
        uint64_t epBit = (board.enPassantSq != SqNone) ? (1ULL << board.enPassantSq) : 0ULL;

        if (us == White) {
            // Left Capture (+7)
            uint64_t capLeft = (pawns & notAFile) << 7;
            uint64_t hitsLeft = capLeft & (enemies | epBit);
            while (hitsLeft) {
                int to = Bitboard::getLsb(hitsLeft);
                int from = to - 7;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to >= 56) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsLeft);
            }
            // Right Capture (+9)
            uint64_t capRight = (pawns & notHFile) << 9;
            uint64_t hitsRight = capRight & (enemies | epBit);
            while (hitsRight) {
                int to = Bitboard::getLsb(hitsRight);
                int from = to - 9;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to >= 56) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsRight);
            }
        } else {
            // Black Left Capture (-9)
            uint64_t capLeft = (pawns & notAFile) >> 9;
            uint64_t hitsLeft = capLeft & (enemies | epBit);
            while (hitsLeft) {
                int to = Bitboard::getLsb(hitsLeft);
                int from = to + 9;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to <= 7) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsLeft);
            }
            // Black Right Capture (-7)
            uint64_t capRight = (pawns & notHFile) >> 7;
            uint64_t hitsRight = capRight & (enemies | epBit);
            while (hitsRight) {
                int to = Bitboard::getLsb(hitsRight);
                int from = to + 7;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to <= 7) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsRight);
            }
        }
    }

    void MoveGen::addPromotions(int from, int to, MoveList& moves, bool capture) {
        if (capture) {
            moves.add(Move(from, to, PromotionQueenCapture));
            moves.add(Move(from, to, PromotionRookCapture));
            moves.add(Move(from, to, PromotionBishopCapture));
            moves.add(Move(from, to, PromotionKnightCapture));
        } else {
            moves.add(Move(from, to, PromotionQueen));
            moves.add(Move(from, to, PromotionRook));
            moves.add(Move(from, to, PromotionBishop));
            moves.add(Move(from, to, PromotionKnight));
        }
    }
   
    
    // Piece Moves (Simplified using Bitboard Lookups)
    
    void MoveGen::generateKnightMoves(const Board& board, MoveList& moveList) {
        uint64_t knights = board.getPieces(Knight, board.sideToMove);
        uint64_t friends = board.occupancy[board.sideToMove];
        
        while (knights) {
            int from = Bitboard::getLsb(knights);
            uint64_t attacks = Bitboard::getKnightAttacks(from) & ~friends;
            
            while (attacks) {
                int to = Bitboard::getLsb(attacks);
                bool capture = board.occupancy[1 - board.sideToMove] & (1ULL << to);
                moveList.add(Move(from, to, capture ? Capture : Quiet));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(knights);
        }
    }

    void MoveGen::generateKingMoves(const Board& board, MoveList& moveList) {
        uint64_t king = board.getPieces(King, board.sideToMove);
        uint64_t friends = board.occupancy[board.sideToMove];
        
        if (king) {
            int from = Bitboard::getLsb(king);
            uint64_t attacks = Bitboard::getKingAttacks(from) & ~friends;
            
            while (attacks) {
                int to = Bitboard::getLsb(attacks);
                bool capture = board.occupancy[1 - board.sideToMove] & (1ULL << to);
                moveList.add(Move(from, to, capture ? Capture : Quiet));
                Bitboard::popLsb(attacks);
            }
        }
    }

    void MoveGen::generateBishopMoves(const Board& board, MoveList& moveList) {
        uint64_t bishops = board.getPieces(Bishop, board.sideToMove);
        uint64_t friends = board.occupancy[board.sideToMove];
        uint64_t occ = board.getAllPieces();
        
        while (bishops) {
            int from = Bitboard::getLsb(bishops);
            uint64_t attacks = Bitboard::getBishopAttacks(from, occ) & ~friends;
            
            while (attacks) {
                int to = Bitboard::getLsb(attacks);
                bool capture = board.occupancy[1 - board.sideToMove] & (1ULL << to);
                moveList.add(Move(from, to, capture ? Capture : Quiet));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(bishops);
        }
    }

    void MoveGen::generateRookMoves(const Board& board, MoveList& moveList) {
        uint64_t rooks = board.getPieces(Rook, board.sideToMove);
        uint64_t friends = board.occupancy[board.sideToMove];
        uint64_t occ = board.getAllPieces();
        
        while (rooks) {
            int from = Bitboard::getLsb(rooks);
            uint64_t attacks = Bitboard::getRookAttacks(from, occ) & ~friends;
            
            while (attacks) {
                int to = Bitboard::getLsb(attacks);
                bool capture = board.occupancy[1 - board.sideToMove] & (1ULL << to);
                moveList.add(Move(from, to, capture ? Capture : Quiet));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(rooks);
        }
    }

    void MoveGen::generateQueenMoves(const Board& board, MoveList& moveList) {
        uint64_t queens = board.getPieces(Queen, board.sideToMove);
        uint64_t friends = board.occupancy[board.sideToMove];
        uint64_t occ = board.getAllPieces();
        
        while (queens) {
            int from = Bitboard::getLsb(queens);
            // Queen is just Bishop + Rook
            uint64_t attacks = (Bitboard::getBishopAttacks(from, occ) | Bitboard::getRookAttacks(from, occ)) & ~friends;
            
            while (attacks) {
                int to = Bitboard::getLsb(attacks);
                bool capture = board.occupancy[1 - board.sideToMove] & (1ULL << to);
                moveList.add(Move(from, to, capture ? Capture : Quiet));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(queens);
        }
    }

    // Castling
    void MoveGen::generateCastlingMoves(const Board& board, MoveList& moveList) {
        uint64_t occ = board.getAllPieces();
        
        if (board.sideToMove == White) {
            // White King Side 
            if ((board.castlingRights & CastlingWhiteKingSide) && 
                !Bitboard::getBit(occ, SqF1) && !Bitboard::getBit(occ, SqG1)) {
                // Ideally check isSquareAttacked(F1) etc. here
                moveList.add(Move(SqE1, SqG1, KingCastle));
            }
            // White Queen Side 
            if ((board.castlingRights & CastlingWhiteQueenSide) && 
                !Bitboard::getBit(occ, SqD1) && !Bitboard::getBit(occ, SqC1) && !Bitboard::getBit(occ, SqB1)) {
                moveList.add(Move(SqE1, SqC1, QueenCastle));
            }
        } else {
            // Black King Side 
            if ((board.castlingRights & CastlingBlackKingSide) && 
                !Bitboard::getBit(occ, SqF8) && !Bitboard::getBit(occ, SqG8)) {
                moveList.add(Move(SqE8, SqG8, KingCastle));
            }
            // Black Queen Side 
            if ((board.castlingRights & CastlingBlackQueenSide) && 
                !Bitboard::getBit(occ, SqD8) && !Bitboard::getBit(occ, SqC8) && !Bitboard::getBit(occ, SqB8)) {
                moveList.add(Move(SqE8, SqC8, QueenCastle));
            }
        }
    }
    void MoveGen::generateCaptures(const Board& board, MoveList& moveList) {
        moveList.reset();
        int us = board.sideToMove;
        int them = 1 - us;
        uint64_t enemies = board.occupancy[them];
        uint64_t empty = ~board.getAllPieces();
        uint64_t occ = board.getAllPieces();

        uint64_t pawns = board.getPieces(Pawn, us);
        int up = (us == White) ? 8 : -8;
        
        uint64_t singlePush = (us == White) ? (pawns << 8) : (pawns >> 8);
        singlePush &= empty;
        uint64_t promPushes = singlePush & ((us == White) ? 0xFF00000000000000ULL : 0x00000000000000FFULL);
        while(promPushes) {
            int to = Bitboard::getLsb(promPushes);
            moveList.add(Move(to - up, to, PromotionQueen));
            moveList.add(Move(to - up, to, PromotionRook));
            moveList.add(Move(to - up, to, PromotionBishop));
            moveList.add(Move(to - up, to, PromotionKnight));
            Bitboard::popLsb(promPushes);
        }

        const uint64_t notAFile = 0xFEFEFEFEFEFEFEFEULL;
        const uint64_t notHFile = 0x7F7F7F7F7F7F7F7FULL;
        uint64_t epBit = (board.enPassantSq != SqNone) ? (1ULL << board.enPassantSq) : 0ULL;

        if (us == White) {
            uint64_t capLeft = (pawns & notAFile) << 7;
            uint64_t hitsLeft = capLeft & (enemies | epBit);
            while (hitsLeft) {
                int to = Bitboard::getLsb(hitsLeft);
                int from = to - 7;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to >= 56) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsLeft);
            }
            uint64_t capRight = (pawns & notHFile) << 9;
            uint64_t hitsRight = capRight & (enemies | epBit);
            while (hitsRight) {
                int to = Bitboard::getLsb(hitsRight);
                int from = to - 9;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to >= 56) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsRight);
            }
        } else {
            uint64_t capLeft = (pawns & notAFile) >> 9;
            uint64_t hitsLeft = capLeft & (enemies | epBit);
            while (hitsLeft) {
                int to = Bitboard::getLsb(hitsLeft);
                int from = to + 9;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to <= 7) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsLeft);
            }
            uint64_t capRight = (pawns & notHFile) >> 7;
            uint64_t hitsRight = capRight & (enemies | epBit);
            while (hitsRight) {
                int to = Bitboard::getLsb(hitsRight);
                int from = to + 7;
                if (to == board.enPassantSq) moveList.add(Move(from, to, EpCapture));
                else if (to <= 7) addPromotions(from, to, moveList, true);
                else moveList.add(Move(from, to, Capture));
                Bitboard::popLsb(hitsRight);
            }
        }

        uint64_t knights = board.getPieces(Knight, us);
        while (knights) {
            int from = Bitboard::getLsb(knights);
            uint64_t attacks = Bitboard::getKnightAttacks(from) & enemies; 
            while (attacks) {
                moveList.add(Move(from, Bitboard::getLsb(attacks), Capture));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(knights);
        }

        uint64_t bishops = board.getPieces(Bishop, us);
        while (bishops) {
            int from = Bitboard::getLsb(bishops);
            uint64_t attacks = Bitboard::getBishopAttacks(from, occ) & enemies;
            while (attacks) {
                moveList.add(Move(from, Bitboard::getLsb(attacks), Capture));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(bishops);
        }

        uint64_t rooks = board.getPieces(Rook, us);
        while (rooks) {
            int from = Bitboard::getLsb(rooks);
            uint64_t attacks = Bitboard::getRookAttacks(from, occ) & enemies;
            while (attacks) {
                moveList.add(Move(from, Bitboard::getLsb(attacks), Capture));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(rooks);
        }

        uint64_t queens = board.getPieces(Queen, us);
        while (queens) {
            int from = Bitboard::getLsb(queens);
            uint64_t attacks = (Bitboard::getBishopAttacks(from, occ) | Bitboard::getRookAttacks(from, occ)) & enemies;
            while (attacks) {
                moveList.add(Move(from, Bitboard::getLsb(attacks), Capture));
                Bitboard::popLsb(attacks);
            }
            Bitboard::popLsb(queens);
        }

        uint64_t king = board.getPieces(King, us);
        if (king) {
            int from = Bitboard::getLsb(king);
            uint64_t attacks = Bitboard::getKingAttacks(from) & enemies;
            while (attacks) {
                moveList.add(Move(from, Bitboard::getLsb(attacks), Capture));
                Bitboard::popLsb(attacks);
            }
        }
    }
}