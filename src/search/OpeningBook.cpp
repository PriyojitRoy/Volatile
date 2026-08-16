#include "search/OpeningBook.hpp"
#include "core/Bitboard.hpp"
#include "core/MoveGen.hpp"
#include "core/ConstantArrays.hpp"
#include <fstream>
#define U64(x) x##ULL

namespace VEngine {

    bool OpeningBook::hasBook = false;
    std::vector<PolyglotEntry> OpeningBook::entries;


    bool OpeningBook::load(const std::vector<std::string>& filenames) {
        entries.clear();

        for (const std::string& filename : filenames) {
            std::ifstream file;
            std::vector<std::string> searchPaths = {
                "data/books/",
                "../data/books/",
                "../../data/books/"
            };
            
            bool opened = false;
            for (const auto& path : searchPaths) {
                file.open(path + filename, std::ios::binary);
                if (file.is_open()) {
                    opened = true;
                    break;
                }
            }

            if (!opened) {
                std::cout << "Warning: Could not open book file " << filename << std::endl;
                continue;
            }

            file.seekg(0, std::ios::end);
            size_t size = file.tellg();
            file.seekg(0, std::ios::beg);

            int numEntries = size / sizeof(PolyglotEntry);
            
            std::vector<PolyglotEntry> tempEntries(numEntries);
            file.read(reinterpret_cast<char*>(tempEntries.data()), size);
            file.close();

            for (auto& entry : tempEntries) {
                entry.key = __builtin_bswap64(entry.key);
                entry.move = __builtin_bswap16(entry.move);
                entry.weight = __builtin_bswap16(entry.weight);
                entry.learn = __builtin_bswap32(entry.learn);
                entries.push_back(entry);
            }
            std::cout << "Loaded " << numEntries << " positions from " << filename << std::endl;
        }

        if (entries.empty()) {
            hasBook = false;
            return false;
        }

        std::cout << "Sorting combined opening book... " << std::flush;
        std::sort(entries.begin(), entries.end(), [](const PolyglotEntry& a, const PolyglotEntry& b) {
            return a.key < b.key;
        });
        std::cout << "Done." << std::endl;
        
        hasBook = true;
        Board testBoard; 
        uint64_t startHash = computePolyglotHash(testBoard);
        std::cout << "info string Startup Polyglot Hash: " << std::hex << startHash << std::dec << std::endl;
        // if (startHash == 0x463b96181691fc9cULL) {
        //     std::cout << "info string [SUCCESS] Polyglot Hashing is PERFECT!" << std::endl;
        // } else {
        //     std::cout << "info string [ERROR] Hash mismatch! Expected 463b96181691fc9c" << std::endl;
        // }
        return true;
    }
    
    Move OpeningBook::getBookMove(Board& board) {
        if (!hasBook || entries.empty()) return Move();

        uint64_t key = computePolyglotHash(board);

        int low = 0, high = entries.size() - 1;
        int firstMatch = -1;

        while (low <= high) {
            int mid = low + (high - low) / 2;
            if (entries[mid].key < key) {
                low = mid + 1;
            } else if (entries[mid].key > key) {
                high = mid - 1;
            } else {
                firstMatch = mid;
                high = mid - 1; 
            }
        }

        if (firstMatch == -1) {
            //std::cout << "info string Hash not found in book." << std::endl;
            return Move(); 
        }

        std::vector<uint16_t> bookMoves;
        std::vector<int> weights;
        int totalWeight = 0;

        for (size_t i = firstMatch; i < entries.size() && entries[i].key == key; i++) {
            int w = entries[i].weight;
            if (w == 0) w = 1; 

            bookMoves.push_back(entries[i].move);
            weights.push_back(w);
            totalWeight += w;
        }

        if (bookMoves.empty() || totalWeight == 0) return Move();

        thread_local static std::random_device rd;
        thread_local static std::mt19937 rng(rd()); 
        std::uniform_int_distribution<int> dist(0, totalWeight - 1);
        int r = dist(rng);
        
        int sum = 0;
        uint16_t chosenPolyMove = bookMoves[0];
        for (size_t i = 0; i < bookMoves.size(); i++) {
            sum += weights[i];
            if (r < sum) {
                chosenPolyMove = bookMoves[i];
                break;
            }
        }

        int toFile = chosenPolyMove & 7;
        int toRank = (chosenPolyMove >> 3) & 7;
        int fromFile = (chosenPolyMove >> 6) & 7;
        int fromRank = (chosenPolyMove >> 9) & 7;
        int promoCode = (chosenPolyMove >> 12) & 7; 

        int fromSq = fromRank * 8 + fromFile;
        int toSq = toRank * 8 + toFile;

        if (board.getPieceAt(fromSq) == King) {
            if (fromSq == 4 && toSq == 7) toSq = 6;       
            else if (fromSq == 4 && toSq == 0) toSq = 2;  
            else if (fromSq == 60 && toSq == 63) toSq = 62; 
            else if (fromSq == 60 && toSq == 56) toSq = 58; 
        }

        std::cout << "info string Book decoded move from square " << fromSq << " to " << toSq << std::endl;

        MoveList moves;
        MoveGen::generateAllMoves(board, moves);
        
        for (int i = 0; i < moves.size(); i++) {
            Move m = moves[i];
            if (m.getFrom() == fromSq && m.getTo() == toSq) {
                // Promotion matching
                if (promoCode > 0) {
                    int myPromo = m.getFlags();
                    if (promoCode == 1 && myPromo != PromotionKnight && myPromo != PromotionKnightCapture) continue;
                    if (promoCode == 2 && myPromo != PromotionBishop && myPromo != PromotionBishopCapture) continue;
                    if (promoCode == 3 && myPromo != PromotionRook && myPromo != PromotionRookCapture) continue;
                    if (promoCode == 4 && myPromo != PromotionQueen && myPromo != PromotionQueenCapture) continue;
                }
                return m;
            }
        }

        std::cout << "info string FATAL: Book move not found in generated legal moves!" << std::endl;
        return Move(); 
    }

    
    uint64_t OpeningBook::computePolyglotHash(const Board& board) {
        uint64_t hash = 0;
        
        for (int sq = 0; sq < 64; sq++) {
            int piece = board.getPieceAt(sq);
            if (piece == None) continue;
        
            int kind = -1;
            bool isWhite = (board.getPieces(piece, White) & (1ULL << sq));
        
            if (piece == Pawn)   kind = isWhite ? 1 : 0;
            else if (piece == Knight) kind = isWhite ? 3 : 2;
            else if (piece == Bishop) kind = isWhite ? 5 : 4;
            else if (piece == Rook)   kind = isWhite ? 7 : 6;
            else if (piece == Queen)  kind = isWhite ? 9 : 8;
            else if (piece == King)   kind = isWhite ? 11 : 10;
        
            if (kind != -1) {
                hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[64 * kind + sq];
            }
        }
    
        if (board.castlingRights & CastlingWhiteKingSide)  hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[768];
        if (board.castlingRights & CastlingWhiteQueenSide) hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[769];
        if (board.castlingRights & CastlingBlackKingSide)  hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[770];
        if (board.castlingRights & CastlingBlackQueenSide) hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[771];
    
        if (board.enPassantSq != SqNone) {
            int epFile = board.enPassantSq % 8;
            //int epRank = board.enPassantSq / 8;
            bool canCapture = false;
            
            int enemyRank = (board.sideToMove == White) ? 4 : 3;
            auto myPawns = board.getPieces(Pawn, board.sideToMove);
            
            if (epFile > 0 && (myPawns & (1ULL << (enemyRank * 8 + epFile - 1)))) canCapture = true;
            if (epFile < 7 && (myPawns & (1ULL << (enemyRank * 8 + epFile + 1)))) canCapture = true;
        
            if (canCapture) {
                hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[772 + epFile];
            }
        }
    
        if (board.sideToMove == White) {
            hash ^= ConstantArrays::PolyglotData::PolyglotRandoms[780];
        }
    
        return hash;
    }

}