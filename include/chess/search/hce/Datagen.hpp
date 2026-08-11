#ifndef CHESS_SEARCH_DATAGEN_H
#define CHESS_SEARCH_DATAGEN_H
#include "chess/core/Board.hpp"
#include <string>
#include <vector>
#include <cstdint>

namespace ChessCore {

    #pragma pack(push, 1)
    struct PackedEntry {
        uint8_t squares[32]; 
        int16_t score;       
        int8_t result;       
        uint8_t flags;       
    };
    #pragma pack(pop)

    struct TrainingEntry {
        Board board;
        int score;
        int result;
    };

    class Datagen {
    public:
        struct TunableParams {
            double c1;
            double c2;
        };

        static void generateData(int gamesToPlay, const std::string& epdFile, const std::string& outputFile);
        static void runSPSATuner(const std::string& epdFile);

    private:
        static std::vector<std::string> loadEPD(const std::string& filename);
        static int checkGameOver(Board& board);
        static uint8_t getPieceAtSquare(const Board& board, int sq);
        static PackedEntry compressPosition(const TrainingEntry& entry);
        static void writeToBinaryFile(const std::vector<TrainingEntry>& gamePositions, const std::string& outputFile);

        static void applyParams(const TunableParams& p);
        static double playMatch(Board board, const TunableParams& whiteParams, const TunableParams& blackParams);
    };
}

#endif // CHESS_SEARCH_DATAGEN_H