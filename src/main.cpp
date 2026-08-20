#include "uci/UCIHandler.hpp"
#include "core/Board.hpp"
#include "search/TT.hpp"
#include "search/OpeningBook.hpp"
#include "search/Datagen.hpp"

int main(int argc, char* argv[]) {
    VEngine::Board board;
    board.initZobrist();

    if (argc > 1) {
        std::string command = argv[1];
        
        if (command == "datagen") {
            if (argc < 5) {
                std::cout << "Error: Missing arguments." << std::endl;
                std::cout << "Usage: ./Volatile datagen <epd_file> <output_file.bin> <num_games>" << std::endl;
                return 1;
            }

            std::string epdFile = argv[2];
            std::string outputFile = argv[3];
            int numGames = std::stoi(argv[4]);
            
            std::cout << "--- Starting Data Generation ---" << std::endl;
            std::cout << "Opening Book : " << epdFile << std::endl;
            std::cout << "Output File  : " << outputFile << std::endl;
            std::cout << "Number of Games : " << numGames << std::endl;

            VEngine::Datagen::generateData(numGames, epdFile, outputFile);
            
            return 0; 
        }
        else if (command == "tune") {
            if (argc < 3) {
                std::cout << "Error: Missing EPD file for tuning." << std::endl;
                std::cout << "Usage: ./Volatile tune <epd_file>" << std::endl;
                return 1;
            }

            std::string epdFile = argv[2];
            
            std::cout << "--- Starting Native SPSA Tuning ---" << std::endl;
            std::cout << "Opening Book : " << epdFile << std::endl;

            VEngine::Datagen::runSPSATuner(epdFile);
            
            return 0; 
        }
    }

    VEngine::UCIHandler handler;

    std::cout << "Type 'help' to see a list of available commands." << std::endl;

    handler.loop();

    return 0;
}