#ifndef CHESS_MOVE_H
#define CHESS_MOVE_H

#include "common.h"

namespace VEngine {

    enum MoveFlag : uint16_t {
        Quiet = 0,
        DoublePawnPush = 1,
        KingCastle = 2,
        QueenCastle = 3,
        Capture = 4,
        EpCapture = 5,
        PromotionKnight = 8,
        PromotionBishop = 9,
        PromotionRook = 10,
        PromotionQueen = 11,
        PromotionKnightCapture = 12,
        PromotionBishopCapture = 13,
        PromotionRookCapture = 14,
        PromotionQueenCapture = 15
    };

    class Move {
    public:
        Move() : data(0) {}
        Move(uint16_t move) : data(move) {}
        Move(int from, int to, int flags = 0) {
            data = (from & 0x3F) | ((to & 0x3F) << 6) | ((flags & 0xF) << 12);
        }

        inline int getFrom() const {
            return data & 0x3F;
        }

        inline int getTo() const {
            return (data >> 6) & 0x3F;
        }

        inline int getFlags() const {
            return (data >> 12) & 0xF;
        }

        inline bool isCapture() const {
            return data & 0x4000;
        }

        inline bool isPromotion() const {
            return data & 0x8000;
        }

        inline bool isCastling() const {
            int f = getFlags();
            return f == KingCastle || f == QueenCastle;
        }

        inline bool isEnPassant() const {
            return getFlags() == EpCapture;
        }

        inline uint16_t getData() const {
            return data;
        }
        std::string toString() const;

        bool operator==(const Move& other) const {
            return data == other.data;    
        }
        private:
        uint16_t data;
    };
}

#endif // CHESS_MOVE_H