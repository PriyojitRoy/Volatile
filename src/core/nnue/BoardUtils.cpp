#include "core/Board.hpp"
#include "core/Constants.hpp"
#include "core/Move.hpp"
#include "core/Bitboard.hpp"
//#define DEBUG_HASH
namespace VEngine {

    extern uint64_t ZOBRIST_PIECES[6][2][64];
    extern uint64_t ZOBRIST_SIDE;
    extern uint64_t ZOBRIST_CASTLING[16];
    extern uint64_t ZOBRIST_EP[64];
bool Board::isThreefoldRepetition() const {
        if (history.empty()) return false;
        int count = 1;
        // Search back only as far as the last irreversible move (capture or pawn move)
        int start = std::max(0, (int)history.size() - halfMoveClock);
        for (int i = (int)history.size() - 1; i >= start; i--) {
            if (history[i].zorbitKey == zorbitKey) count++;
            if (count >= 3) return true;
        }
        return false;
    }

bool Board::isCheck() {
        int kingSq = getKingSquare(sideToMove);
        if (kingSq == -1) return false;
        return isSquareAttacked(kingSq, 1 - sideToMove);
    }

bool Board::isInsufficientMaterial() const {
        if (pieces[Pawn] || pieces[Rook] || pieces[Queen]) return false;
        int minorCount = Bitboard::countBits(pieces[Knight] | pieces[Bishop]);
        return minorCount <= 1;
    }

bool Board::isSquareAttacked(int sq, int attackerSide) const {
        uint64_t occ = occupancy[Both];
        if (Bitboard::getKnightAttacks(sq) & pieces[Knight] & occupancy[attackerSide]) return true;
        if (Bitboard::getKingAttacks(sq) & pieces[King] & occupancy[attackerSide]) return true;
        
        uint64_t diag = (pieces[Bishop] | pieces[Queen]) & occupancy[attackerSide];
        if (diag && (Bitboard::getBishopAttacks(sq, occ) & diag)) return true;
        
        uint64_t ortho = (pieces[Rook] | pieces[Queen]) & occupancy[attackerSide];
        if (ortho && (Bitboard::getRookAttacks(sq, occ) & ortho)) return true;
        
        if (attackerSide == White) {
            uint64_t pawns = pieces[Pawn] & occupancy[White];
            if (((1ULL << sq) >> 7) & 0xFEFEFEFEFEFEFEFEULL & pawns) return true;
            if (((1ULL << sq) >> 9) & 0x7F7F7F7F7F7F7F7FULL & pawns) return true;
        } else {
            uint64_t pawns = pieces[Pawn] & occupancy[Black];
            if (((1ULL << sq) << 7) & 0x7F7F7F7F7F7F7F7FULL & pawns) return true;
            if (((1ULL << sq) << 9) & 0xFEFEFEFEFEFEFEFEULL & pawns) return true;
        }
        return false;
    }

int Board::getPieceAt(int sq) const {
        if (!Bitboard::getBit(occupancy[Both], sq)) return None;
        for (int i = 0; i < 6; i++) if (Bitboard::getBit(pieces[i], sq)) return i;
        return None;
    }

int Board::getKingSquare(int color) const {
        uint64_t kingBB = pieces[King] & occupancy[color];
        return kingBB ? Bitboard::getLsb(kingBB) : -1;
    }

uint64_t Board::getPieces(int p, int c) const { return pieces[p] & occupancy[c]; }

}
