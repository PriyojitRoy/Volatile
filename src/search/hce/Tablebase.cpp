#include "chess/search/hce/Tablebase.hpp"
#include "chess/core/MoveGen.hpp"
#include "chess/core/Bitboard.hpp"

extern "C" {
    #include "../../fathom/tbprobe.h"
}

namespace ChessCore {
    int Tablebase::MaxPieces = 5;
    bool Tablebase::loaded = false;

    void Tablebase::init(const std::string& path) {
        if (tb_init(path.c_str())) {
            loaded = true;
            std::cout << "info string Syzygy tablebases loaded successfully from " << path << std::endl;
        } else {
            std::cout << "info string Failed to load Syzygy tablebases." << std::endl;
        }
    }

    bool Tablebase::isLoaded() { return loaded; }

    Move Tablebase::probeRoot(const Board& board, int& score) {
        uint64_t white = board.occupancy[White];
        uint64_t black = board.occupancy[Black];
        uint64_t kings = board.getPieces(King, Both);
        uint64_t queens = board.getPieces(Queen, Both);
        uint64_t rooks = board.getPieces(Rook, Both);
        uint64_t bishops = board.getPieces(Bishop, Both);
        uint64_t knights = board.getPieces(Knight, Both);
        uint64_t pawns = board.getPieces(Pawn, Both);
        
        unsigned ep = (board.enPassantSq >= 0 && board.enPassantSq <= 63) ? board.enPassantSq : 0;
        
        unsigned rule50 = board.halfMoveClock;
        unsigned castling = 0; 
        bool turn = (board.sideToMove == White);

        unsigned results[TB_MAX_MOVES];
        
        unsigned res = tb_probe_root(white, black, kings, queens, rooks, bishops, knights, pawns, rule50, castling, ep, turn, results);

        if (res == TB_RESULT_FAILED) {
            return Move(); 
        }

        int from = TB_GET_FROM(res);
        int to = TB_GET_TO(res);
        int promo = TB_GET_PROMOTES(res);

        char moveStr[6];
        moveStr[0] = 'a' + (from % 8);
        moveStr[1] = '1' + (from / 8);
        moveStr[2] = 'a' + (to % 8);
        moveStr[3] = '1' + (to / 8);
        moveStr[4] = '\0';
        
        if (promo != TB_PROMOTES_NONE) {
            if (promo == TB_PROMOTES_QUEEN) moveStr[4] = 'q';
            else if (promo == TB_PROMOTES_ROOK) moveStr[4] = 'r';
            else if (promo == TB_PROMOTES_BISHOP) moveStr[4] = 'b';
            else if (promo == TB_PROMOTES_KNIGHT) moveStr[4] = 'n';
            moveStr[5] = '\0';
        }

        std::string targetMove(moveStr);

        MoveList moves;
        MoveGen::generateAllMoves(const_cast<Board&>(board), moves);

        for (int i = 0; i < moves.size(); i++) {
            Move m = moves.moves[i];
            if (m.toString() == targetMove) {
                int wdl = TB_GET_WDL(res); 
                if (wdl == 1 || wdl == 2) score = 20000;
                else if (wdl == -1 || wdl == -2) score = -20000;
                else score = 0;
                
                return m;
            }
        }

        return Move(); 
    }

    bool Tablebase::probeWDL(const Board& board, int& wdlScore) {
        uint64_t white = board.occupancy[White];
        uint64_t black = board.occupancy[Black];
        uint64_t kings = board.getPieces(King, Both);
        uint64_t queens = board.getPieces(Queen, Both);
        uint64_t rooks = board.getPieces(Rook, Both);
        uint64_t bishops = board.getPieces(Bishop, Both);
        uint64_t knights = board.getPieces(Knight, Both);
        uint64_t pawns = board.getPieces(Pawn, Both);

        if (Bitboard::countBits(kings & white) != 1 || Bitboard::countBits(kings & black) != 1) return false;
        if (pawns & 0xFF000000000000FFULL) return false; 
        
        unsigned ep = (board.enPassantSq >= 0 && board.enPassantSq <= 63) ? board.enPassantSq : 0;
        
        unsigned rule50 = board.halfMoveClock;
        bool turn = (board.sideToMove == White);

        unsigned res = tb_probe_wdl(white, black, kings, queens, rooks, bishops, knights, pawns, rule50, 0, ep, turn);

        if (res == TB_RESULT_FAILED) {
            return false; 
        }

        if (res == 4 || res == 3) wdlScore = 20000;
        else if (res == 0 || res == 1) wdlScore = -20000;
        else wdlScore = 0;

        return true;
    }
}