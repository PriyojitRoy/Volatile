#include "chess/core/Move.hpp"
#include "chess/core/Constants.hpp"

namespace ChessCore {

    std::string squareToString(int sq) {
        if (sq < 0 || sq > 63) return "-";
        int rank = sq / 8;
        int file = sq % 8;
        
        std::string s = "";
        s += ('a' + file);
        s += ('1' + rank);
        return s;
    }

   std::string Move::toString() const {
        if (data == 0) return "0000"; // Null move/None

        int from = getFrom();
        int to = getTo();
        int flags = getFlags();

        // Convert square index to coordinate 
        auto toCoord = [](int sq) -> std::string {
            char file = 'a' + (sq % 8);
            char rank = '1' + (sq / 8);
            return {file, rank};
        };

        std::string moveStr = toCoord(from) + toCoord(to);

        // Handle Promotions: UCI requires a trailing character (q, r, b, n)
        if (isPromotion()) {
            if (flags == PromotionQueen || flags == PromotionQueenCapture) moveStr += 'q';
            else if (flags == PromotionRook || flags == PromotionRookCapture) moveStr += 'r';
            else if (flags == PromotionBishop || flags == PromotionBishopCapture) moveStr += 'b';
            else if (flags == PromotionKnight || flags == PromotionKnightCapture) moveStr += 'n';
        }

        return moveStr;
    }
}