#include "search/hce/Datagen.hpp"
#include <iostream>

namespace VEngine {
    void Datagen::generateData([[maybe_unused]] int numGames, [[maybe_unused]] const std::string& epdFile, [[maybe_unused]] const std::string& outputFile) {
        std::cerr << "Datagen::generateData is not implemented for NNUE mode." << std::endl;
    }

    void Datagen::runSPSATuner([[maybe_unused]] const std::string& epdFile) {
        std::cerr << "Datagen::runSPSATuner is not implemented for NNUE mode." << std::endl;
    }
}
