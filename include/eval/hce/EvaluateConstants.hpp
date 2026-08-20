#ifndef VENGINE_EVAL_HCE_CONSTANTS_H
#define VENGINE_EVAL_HCE_CONSTANTS_H

#include <cstdint>

namespace VEngine {
    extern const int32_t mobilities[5][28];
    extern const int32_t bishop_pair;
    extern const int32_t passed_pawns[64];
    extern const int32_t inner_king_zone_attacks[4];
    extern const int32_t outer_king_zone_attacks[4];
    extern const int32_t doubled_pawn_penalty[8];
    extern const int32_t pawn_storm[64];
    extern const int32_t isolated_pawns[64];
    extern const int32_t threats[6][6];
    extern const int32_t rook_semi_open[2];
    extern const int32_t phalanx_pawns[8];
}

#endif // VENGINE_EVAL_HCE_CONSTANTS_H
