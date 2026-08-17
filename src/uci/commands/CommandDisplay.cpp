#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include "core/Constants.hpp"
#include <iostream>
#include <string>

namespace VEngine {
    namespace Commands {
        
        static std::string generateFen(const Board& board) {
            std::string fen = "";
            for (int r = 7; r >= 0; r--) {
                int empty = 0;
                for (int f = 0; f < 8; f++) {
                    int sq = r * 8 + f;
                    int p = board.getPieceAt(sq);
                    if (p == None) {
                        empty++;
                    } else {
                        if (empty > 0) {
                            fen += std::to_string(empty);
                            empty = 0;
                        }
                        char c = "pnbrqk"[p];
                        if (board.occupancy[White] & (1ULL << sq)) fen += (char)toupper(c);
                        else fen += c;
                    }
                }
                if (empty > 0) fen += std::to_string(empty);
                if (r > 0) fen += "/";
            }
            
            fen += (board.sideToMove == White) ? " w " : " b ";
            
            std::string castling = "";
            if (board.castlingRights & CastlingWhiteKingSide) castling += "K";
            if (board.castlingRights & CastlingWhiteQueenSide) castling += "Q";
            if (board.castlingRights & CastlingBlackKingSide) castling += "k";
            if (board.castlingRights & CastlingBlackQueenSide) castling += "q";
            if (castling.empty()) castling = "-";
            fen += castling + " ";
            
            if (board.enPassantSq != SqNone) {
                char file = 'a' + (board.enPassantSq % 8);
                char rank = '1' + (board.enPassantSq / 8);
                fen += file;
                fen += rank;
            } else {
                fen += "-";
            }
            
            fen += " " + std::to_string(board.halfMoveClock);
            fen += " " + std::to_string(board.fullMoveNumber);
            
            return fen;
        }

        void executeDisplay(UCIHandler& handler, std::istringstream& /*ss*/) {
            handler.board.print(); 
            std::cout << "Side to move: " << (handler.board.sideToMove == White ? "White" : "Black") << "\n";
            std::cout << "Zobrist Key:  " << std::hex << handler.board.zorbitKey << std::dec << "\n";
            std::cout << "Game Phase:   " << handler.board.evalState.phase << "\n";
            std::cout << "FEN:          " << generateFen(handler.board) << "\n" << std::endl;
        }
    }
}
