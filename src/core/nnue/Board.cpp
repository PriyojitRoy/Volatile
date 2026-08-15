#include "core/Board.hpp"
#include "core/Constants.hpp"
#include "core/Move.hpp"
#include "core/Bitboard.hpp"
#include "eval/nnue/Network.hpp"
//#define DEBUG_HASH
namespace VEngine {

    uint64_t ZOBRIST_PIECES[6][2][64];
    uint64_t ZOBRIST_SIDE;
    uint64_t ZOBRIST_CASTLING[16];
    uint64_t ZOBRIST_EP[64];
void Board::initZobrist() {
        std::mt19937_64 rng(123456789);
        for (int p = 0; p < 6; p++)
            for (int c = 0; c < 2; c++)
                for (int s = 0; s < 64; s++)
                    ZOBRIST_PIECES[p][c][s] = rng();

        ZOBRIST_SIDE = rng();
        for (int i = 0; i < 16; i++) ZOBRIST_CASTLING[i] = rng();
        for (int i = 0; i < 64; i++) ZOBRIST_EP[i] = rng();
    }

void Board::initKeys() {
        pawnKey = minorKey = majorKey = 0ULL;

        for (int sq = 0; sq < 64; sq++) {
            int piece = getPieceAt(sq);
            if (piece != None) {
                int color = (occupancy[White] & (1ULL << sq)) ? White : Black;
                if (piece == Pawn) pawnKey ^= ZOBRIST_PIECES[Pawn][color][sq];
                else if (piece == Knight || piece == Bishop) minorKey ^= ZOBRIST_PIECES[piece][color][sq];
                else if (piece == Rook || piece == Queen) majorKey ^= ZOBRIST_PIECES[piece][color][sq];
            }
        }
    }



    Board::Board() {
        reset();
    }
    void Board::reset() {
        for (int i = 0; i < 6; i++) pieces[i] = 0ULL;
        for (int i = 0; i < 3; i++) occupancy[i] = 0ULL;
        sideToMove = White;
        enPassantSq = SqNone;
        castlingRights = 0;
        halfMoveClock = 0;
        fullMoveNumber = 1;
        zorbitKey = 0;
        
        
        
        history.clear();
        accumulator.init(Network::getBiases()); 
        initKeys();
    }

void Board::hashBoard() {
        zorbitKey = 0;
        for (int p = Pawn; p <= King; p++) {
            for (int c = White; c <= Black; c++) {
                uint64_t bb = pieces[p] & occupancy[c];
                while (bb) {
                    int sq = Bitboard::getLsb(bb);
                    zorbitKey ^= ZOBRIST_PIECES[p][c][sq];
                    Bitboard::popLsb(bb);
                }
            }
        }
        if (sideToMove == Black) zorbitKey ^= ZOBRIST_SIDE;
        zorbitKey ^= ZOBRIST_CASTLING[castlingRights];
        if (enPassantSq != SqNone) zorbitKey ^= ZOBRIST_EP[enPassantSq];
    }

bool Board::verifyZobrist() {
        uint64_t currentKey = zorbitKey;
        uint64_t recalculated = 0;
        for (int p = Pawn; p <= King; p++) {
            for (int c = White; c <= Black; c++) {
                uint64_t bb = pieces[p] & occupancy[c];
                while (bb) {
                    int sq = Bitboard::getLsb(bb);
                    recalculated ^= ZOBRIST_PIECES[p][c][sq];
                    Bitboard::popLsb(bb);
                }
            }
        }
        if (sideToMove == Black) recalculated ^= ZOBRIST_SIDE;
        recalculated ^= ZOBRIST_CASTLING[castlingRights];
        if (enPassantSq != SqNone) recalculated ^= ZOBRIST_EP[enPassantSq];
        return currentKey == recalculated;
    }

bool Board::verifyKeys() {
        uint64_t testPawn = 0ULL;
        uint64_t testMinor = 0ULL;
        uint64_t testMajor = 0ULL;

        for (int sq = 0; sq < 64; sq++) {
            int piece = getPieceAt(sq);
            if (piece != None) {
                int color = (occupancy[White] & (1ULL << sq)) ? White : Black;
                
                if (piece == Pawn) {
                    testPawn ^= ZOBRIST_PIECES[Pawn][color][sq];
                } else if (piece == Knight || piece == Bishop) {
                    testMinor ^= ZOBRIST_PIECES[piece][color][sq];
                } else if (piece == Rook || piece == Queen) {
                    testMajor ^= ZOBRIST_PIECES[piece][color][sq];
                }
            }
        }

        bool ok = true;
        if (testPawn != pawnKey) { std::cout << "PawnKey mismatch!\n"; ok = false; }
        if (testMinor != minorKey) { std::cout << "MinorKey mismatch!\n"; ok = false; }
        if (testMajor != majorKey) { std::cout << "MajorKey mismatch!\n"; ok = false; }
        
        return ok;
    }

void Board::print() {
        std::cout << "\n";
        for (int r = 7; r >= 0; r--) {
            std::cout << r + 1 << "  ";
            for (int f = 0; f < 8; f++) {
                int sq = r * 8 + f;
                int p = getPieceAt(sq);
                if (p == None) std::cout << ". ";
                else {
                    char c = "pnbrqk"[p];
                    if (occupancy[White] & (1ULL << sq)) std::cout << (char)toupper(c) << " ";
                    else std::cout << (char)tolower(c) << " ";
                }
            }
            std::cout << "\n";
        }
        std::cout << "   a b c d e f g h\n\n";
    }

void Board::parseFen(const std::string& fen) {
        reset(); 
        std::istringstream ss(fen);
        std::string pos, side, castling, ep, half, full;
        if (!(ss >> pos >> side >> castling >> ep)) return;
        if (!(ss >> half >> full)) { half = "0"; full = "1"; }

        int r = 7, f = 0;
        for (char c : pos) {
            if (c == '/') { r--; f = 0; }
            else if (isdigit(c)) { f += (c - '0'); }
            else {
                int sq = r * 8 + f;
                int color = isupper(c) ? White : Black;
                char type = tolower(c);
                int p = (type == 'p') ? Pawn : (type == 'n') ? Knight : (type == 'b') ? Bishop : (type == 'r') ? Rook : (type == 'q') ? Queen : King;
                Bitboard::setBit(pieces[p], sq);
                Bitboard::setBit(occupancy[color], sq);
                f++;
            }
        }
        occupancy[Both] = occupancy[White] | occupancy[Black];
        sideToMove = (side == "w") ? White : Black;
        castlingRights = 0;
        if (castling != "-") {
            for (char c : castling) {
                if (c == 'K') castlingRights |= CastlingWhiteKingSide;
                else if (c == 'Q') castlingRights |= CastlingWhiteQueenSide;
                else if (c == 'k') castlingRights |= CastlingBlackKingSide;
                else if (c == 'q') castlingRights |= CastlingBlackQueenSide;
            }
        }
        if (ep != "-") {
            int file = ep[0] - 'a', rank = ep[1] - '1';
            enPassantSq = rank * 8 + file;
        }
        halfMoveClock = std::stoi(half);
        fullMoveNumber = std::stoi(full);
        hashBoard();
        initKeys(); 
        
        
        
            
        for (int p = Pawn; p <= King; p++) {
            for (int c = White; c <= Black; c++) {
                uint64_t bb = pieces[p] & occupancy[c];
                while (bb) {
                    int sq = Bitboard::getLsb(bb);
                    Bitboard::popLsb(bb);
                
                    [[maybe_unused]] int tableSq = (c == White) ? (sq ^ 56) : sq;
                
                    
                    
                    
                }
            }
        }
    }

}
