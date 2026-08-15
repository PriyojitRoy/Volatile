#include "core/Board.hpp"
#include "core/Constants.hpp"
#include "core/Move.hpp"
#include "core/Bitboard.hpp"
#include "eval/hce/Evaluate.hpp"
//#define DEBUG_HASH
#define S(mg, eg) ((int32_t)(((uint32_t)(uint16_t)(eg) << 16) | (uint16_t)(mg)))
inline int32_t unpack_mg(int32_t s) { return (int16_t)(s & 0xFFFF); }
inline int32_t unpack_eg(int32_t s) { return (int16_t)((s >> 16) & 0xFFFF); }

namespace VEngine {

    extern const int32_t PieceValueMG[6];
    extern const int32_t PieceValueEG[6];
    extern const int32_t game_phase_increment[6];
    extern uint64_t ZOBRIST_PIECES[6][2][64];
    extern uint64_t ZOBRIST_SIDE;
    extern uint64_t ZOBRIST_CASTLING[16];
    extern uint64_t ZOBRIST_EP[64];
void Board::makeNullMove() {
        history.push_back({
            castlingRights, 
            enPassantSq, 
            halfMoveClock, 
            zorbitKey, 
            None, 
            None,
            evalState,
            pawnKey,
            minorKey,
            majorKey
        });
        
        zorbitKey ^= ZOBRIST_SIDE;
        if (enPassantSq != SqNone) zorbitKey ^= ZOBRIST_EP[enPassantSq];
        
        enPassantSq = SqNone;
        halfMoveClock++;
        if (sideToMove == Black) fullMoveNumber++;
        sideToMove = 1 - sideToMove;
    }

void Board::unmakeNullMove() {
        if (history.empty()) return;
        GameState state = history.back();
        history.pop_back();

        sideToMove = 1 - sideToMove;
        if (sideToMove == Black) fullMoveNumber--;

        enPassantSq = state.enPassantSq;
        halfMoveClock = state.halfMoveClock;
        zorbitKey = state.zorbitKey; 
    }

bool Board::makeMove(Move move) {
        const int from = move.getFrom();
        const int to = move.getTo();
        const int flags = move.getFlags();
        const int piece = getPieceAt(from);
        
        if (piece == None) return false;

        if (flags == KingCastle) {
            if (isSquareAttacked(from, 1 - sideToMove)) return false;
            int transitSq = (sideToMove == White) ? SqF1 : SqF8;
            if (isSquareAttacked(transitSq, 1 - sideToMove)) return false;
        }
        else if (flags == QueenCastle) {
            if (isSquareAttacked(from, 1 - sideToMove)) return false;
            int transitSq = (sideToMove == White) ? SqD1 : SqD8;
            if (isSquareAttacked(transitSq, 1 - sideToMove)) return false;
        }

        history.push_back({
            castlingRights, 
            enPassantSq, 
            halfMoveClock, 
            zorbitKey, 
            None,
            piece,
            evalState, 
            pawnKey, 
            minorKey, 
            majorKey
        });

        int captured = getPieceAt(to);
        int movingColor = sideToMove;

        zorbitKey ^= ZOBRIST_SIDE;
        zorbitKey ^= ZOBRIST_CASTLING[castlingRights];
        if (enPassantSq != SqNone) zorbitKey ^= ZOBRIST_EP[enPassantSq];

        zorbitKey ^= ZOBRIST_PIECES[piece][movingColor][from];
        if (piece == Pawn) {
            pawnKey ^= ZOBRIST_PIECES[Pawn][movingColor][from];
        } else if (piece == Knight || piece == Bishop) {
            minorKey ^= ZOBRIST_PIECES[piece][movingColor][from];
        } else if (piece == Rook || piece == Queen) {
            majorKey ^= ZOBRIST_PIECES[piece][movingColor][from];
        }

        int tableFrom = (movingColor == White) ? (from ^ 56) : from;
        evalState.mg[movingColor] -= PieceValueMG[piece] + unpack_mg(PSQT[piece][tableFrom]);
        evalState.eg[movingColor] -= PieceValueEG[piece] + unpack_eg(PSQT[piece][tableFrom]);
        evalState.phase -= game_phase_increment[piece];
        
        Bitboard::popBit(pieces[piece], from);
        Bitboard::popBit(occupancy[movingColor], from);

        if (flags == EpCapture) {
            int capColor = 1 - movingColor;
            int capSq = (movingColor == White) ? to - 8 : to + 8;
            int tableCap = (capColor == White) ? (capSq ^ 56) : capSq;

            evalState.mg[capColor] -= PieceValueMG[Pawn] + unpack_mg(PSQT[Pawn][tableCap]);
            evalState.eg[capColor] -= PieceValueEG[Pawn] + unpack_eg(PSQT[Pawn][tableCap]);
            evalState.phase -= game_phase_increment[Pawn];
            captured = Pawn;

            Bitboard::popBit(pieces[Pawn], capSq);
            Bitboard::popBit(occupancy[capColor], capSq);

            zorbitKey ^= ZOBRIST_PIECES[Pawn][capColor][capSq];
            pawnKey   ^= ZOBRIST_PIECES[Pawn][capColor][capSq];
        }
        else if (captured != None) {
            int capColor = 1 - movingColor;
            int tableCap = (capColor == White) ? (to ^ 56) : to;

            evalState.mg[capColor] -= PieceValueMG[captured] + unpack_mg(PSQT[captured][tableCap]);
            evalState.eg[capColor] -= PieceValueEG[captured] + unpack_eg(PSQT[captured][tableCap]);
            evalState.phase -= game_phase_increment[captured];

            Bitboard::popBit(pieces[captured], to);
            Bitboard::popBit(occupancy[capColor], to);

            zorbitKey ^= ZOBRIST_PIECES[captured][capColor][to];
            if (captured == Pawn) {
                pawnKey ^= ZOBRIST_PIECES[Pawn][capColor][to];
            } else if (captured == Knight || captured == Bishop) {
                minorKey ^= ZOBRIST_PIECES[captured][capColor][to];
            } else if (captured == Rook || captured == Queen) {
                majorKey ^= ZOBRIST_PIECES[captured][capColor][to];
            }
        }

        history.back().capturedPiece = captured;

        int placedPiece = piece;
        if (move.isPromotion()) {
            if (flags == PromotionKnight || flags == PromotionKnightCapture) placedPiece = Knight;
            else if (flags == PromotionBishop || flags == PromotionBishopCapture) placedPiece = Bishop;
            else if (flags == PromotionRook || flags == PromotionRookCapture) placedPiece = Rook;
            else placedPiece = Queen;
        }

        Bitboard::setBit(pieces[placedPiece], to);
        Bitboard::setBit(occupancy[movingColor], to);

        zorbitKey ^= ZOBRIST_PIECES[placedPiece][movingColor][to];
        if (placedPiece == Pawn) {
            pawnKey ^= ZOBRIST_PIECES[Pawn][movingColor][to];
        } else if (placedPiece == Knight || placedPiece == Bishop) {
            minorKey ^= ZOBRIST_PIECES[placedPiece][movingColor][to];
        } else if (placedPiece == Rook || placedPiece == Queen) {
            majorKey ^= ZOBRIST_PIECES[placedPiece][movingColor][to];
        }

        int tableTo = (movingColor == White) ? (to ^ 56) : to;
        evalState.mg[movingColor] += PieceValueMG[placedPiece] + unpack_mg(PSQT[placedPiece][tableTo]);
        evalState.eg[movingColor] += PieceValueEG[placedPiece] + unpack_eg(PSQT[placedPiece][tableTo]);
        evalState.phase += game_phase_increment[placedPiece];

        if (flags == KingCastle || flags == QueenCastle) {
            int rF, rT;
            if (flags == KingCastle) {
                rF = (movingColor == White) ? SqH1 : SqH8;
                rT = (movingColor == White) ? SqF1 : SqF8;
            } else {
                rF = (movingColor == White) ? SqA1 : SqA8;
                rT = (movingColor == White) ? SqD1 : SqD8;
            }

            int tableRf = (movingColor == White) ? (rF ^ 56) : rF;
            int tableRt = (movingColor == White) ? (rT ^ 56) : rT;

            evalState.mg[movingColor] -= (PieceValueMG[Rook] + unpack_mg(PSQT[Rook][tableRf]));
            evalState.eg[movingColor] -= (PieceValueEG[Rook] + unpack_eg(PSQT[Rook][tableRf]));
            evalState.mg[movingColor] += (PieceValueMG[Rook] + unpack_mg(PSQT[Rook][tableRt]));
            evalState.eg[movingColor] += (PieceValueEG[Rook] + unpack_eg(PSQT[Rook][tableRt]));

            Bitboard::popBit(pieces[Rook], rF); Bitboard::popBit(occupancy[movingColor], rF);
            Bitboard::setBit(pieces[Rook], rT); Bitboard::setBit(occupancy[movingColor], rT);

            uint64_t rookXor = ZOBRIST_PIECES[Rook][movingColor][rF] ^ ZOBRIST_PIECES[Rook][movingColor][rT];
            zorbitKey ^= rookXor;
            majorKey  ^= rookXor;
        }

        if (piece == Pawn || captured != None) halfMoveClock = 0;
        else halfMoveClock++;

        enPassantSq = SqNone;
        if (piece == Pawn && flags == DoublePawnPush)
            enPassantSq = (movingColor == White) ? from + 8 : from - 8;

        if (piece == King)
            castlingRights &= (movingColor == White)
                ? ~(CastlingWhiteKingSide | CastlingWhiteQueenSide)
                : ~(CastlingBlackKingSide | CastlingBlackQueenSide);

        if (from == SqA1 || to == SqA1) castlingRights &= ~CastlingWhiteQueenSide;
        if (from == SqH1 || to == SqH1) castlingRights &= ~CastlingWhiteKingSide;
        if (from == SqA8 || to == SqA8) castlingRights &= ~CastlingBlackQueenSide;
        if (from == SqH8 || to == SqH8) castlingRights &= ~CastlingBlackKingSide;

        occupancy[Both] = occupancy[White] | occupancy[Black];

        if (movingColor == Black) fullMoveNumber++;
        sideToMove = 1 - movingColor;

        zorbitKey ^= ZOBRIST_CASTLING[castlingRights];
        if (enPassantSq != SqNone) zorbitKey ^= ZOBRIST_EP[enPassantSq];

        if (isSquareAttacked(getKingSquare(movingColor), 1 - movingColor)) {
            unmakeMove(move);
            return false;
        }

        #ifdef DEBUG_HASH
        if (!verifyZobrist()) {
            std::cout << "Zobrist Key mismatch after makeMove!\n";
            print();
        }
        if(!verifyKeys()){
            std::cout << "Major, Minor or Pawn Key mismatch after makeMove!\n";
        }
        #endif

        return true;
    }

void Board::unmakeMove(Move move) {
        if (history.empty()) return;

        GameState state = history.back();
        history.pop_back();

        evalState = state.evalState;
        castlingRights = state.castlingRights;
        enPassantSq = state.enPassantSq;
        halfMoveClock = state.halfMoveClock;
        zorbitKey = state.zorbitKey;
        
        pawnKey = state.pawnKey;
        minorKey = state.minorKey;
        majorKey = state.majorKey;

        sideToMove = 1 - sideToMove;
        if (sideToMove == Black) fullMoveNumber--;

        const int from = move.getFrom();
        const int to = move.getTo();
        const int flags = move.getFlags();

        int placedPiece = state.movedPiece;
        if (move.isPromotion()) {
            if (flags == PromotionKnight || flags == PromotionKnightCapture) placedPiece = Knight;
            else if (flags == PromotionBishop || flags == PromotionBishopCapture) placedPiece = Bishop;
            else if (flags == PromotionRook || flags == PromotionRookCapture) placedPiece = Rook;
            else placedPiece = Queen;
        }

        Bitboard::popBit(pieces[placedPiece], to);
        Bitboard::popBit(occupancy[sideToMove], to);

        Bitboard::setBit(pieces[state.movedPiece], from);
        Bitboard::setBit(occupancy[sideToMove], from);

        if (state.capturedPiece != None) {
            int capSq = to;
            if (flags == EpCapture)
                capSq = (sideToMove == White) ? to - 8 : to + 8;

            Bitboard::setBit(pieces[state.capturedPiece], capSq);
            Bitboard::setBit(occupancy[1 - sideToMove], capSq);
        }

        if (flags == KingCastle) {
            int rF = (sideToMove == White) ? SqH1 : SqH8;
            int rT = (sideToMove == White) ? SqF1 : SqF8;
            Bitboard::popBit(pieces[Rook], rT); Bitboard::popBit(occupancy[sideToMove], rT);
            Bitboard::setBit(pieces[Rook], rF); Bitboard::setBit(occupancy[sideToMove], rF);
        }
        else if (flags == QueenCastle) {
            int rF = (sideToMove == White) ? SqA1 : SqA8;
            int rT = (sideToMove == White) ? SqD1 : SqD8;
            Bitboard::popBit(pieces[Rook], rT); Bitboard::popBit(occupancy[sideToMove], rT);
            Bitboard::setBit(pieces[Rook], rF); Bitboard::setBit(occupancy[sideToMove], rF);
        }

        occupancy[Both] = occupancy[White] | occupancy[Black];

        #ifdef DEBUG_HASH
        if (!verifyZobrist()) {
            std::cout << "Zobrist mismatch after unmakeMove!\n";
            print();
        }
        if (!verifyKeys()) {
            std::cout << "Keys mismatch after unmakeMove!\n";
            print();
        }
        #endif
    }

}
