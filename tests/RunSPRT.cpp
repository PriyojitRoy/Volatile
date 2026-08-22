#include <iostream>
#include <string>
#include <cstdlib>
#include <filesystem>
#include <vector>
#include <sstream>

namespace fs = std::filesystem;

// ========================================================================
// SPRT Configuration
// Edit these values to change the test parameters.
// ========================================================================
struct SPRTConfig {
    // Engine Paths
    std::string testEnginePath = "../build/Volatile";       // The engine with your new changes
    std::string baseEnginePath = "../build_base/Volatile";  // The unmodified baseline engine
    
    // Testing Framework Selection
    bool useFastChess = true;                                                     // Set to true to use fastchess, false to use cutechess
    std::string runnerPath = "fastchess";                                         // Set the path to the cutechess/fastchess executable
    
    // Opening Book (Using the local UHO dataset)
    std::string openingBook = "../uho/Lichess/UHO_Lichess_4852_v1.epd.part_aa";
    
    // Match Parameters
    std::string timeControl = "10+0.1";
    int concurrency = 4;
    int games = 5000;
    
    // SPRT Bounds (Testing for a strict Elo improvement)
    double elo0 = 0.0;
    double elo1 = 5.0;
    double alpha = 0.05;
    double beta = 0.05;
};

// ========================================================================
// Utility Functions
// ========================================================================
bool fileExists(const std::string& path) {
    return fs::exists(path);
}

bool checkRunner(const std::string& command) {
    std::string checkCmd = command + " --version > /dev/null 2>&1";
    // If running in Windows, suppressing output is slightly different, 
    // but std::system with > /dev/null often works if running under bash/msys, 
    // or we just trust the user on Windows.
#ifdef _WIN32
    checkCmd = command + " --version > nul 2>&1";
#endif
    return std::system(checkCmd.c_str()) == 0;
}

// ========================================================================
// Main Execution
// ========================================================================
int main(int argc, char* argv[]) {
    SPRTConfig config;
    
    std::cout << "===================================================" << std::endl;
    std::cout << " VEngine SPRT Testing Framework" << std::endl;
    std::cout << "===================================================" << std::endl;

    // 1. Validate the test runner installation
    if (!checkRunner(config.runnerPath)) {
        std::cerr << "[ERROR] '" << config.runnerPath << "' could not be found or executed." << std::endl;
        std::cerr << "        Please install fastchess or cutechess-cli system-wide," << std::endl;
        std::cerr << "        OR update the 'runnerPath' variable in tests/RunSPRT.cpp." << std::endl;
        return 1;
    }
    
    // 2. Validate Engine Binaries
    if (!fileExists(config.testEnginePath)) {
        std::cerr << "[ERROR] Test engine not found at: " << fs::absolute(config.testEnginePath) << std::endl;
        std::cerr << "        Make sure you compile your new changes into the 'build' folder." << std::endl;
        return 1;
    }
    if (!fileExists(config.baseEnginePath)) {
        std::cerr << "[ERROR] Base engine not found at: " << fs::absolute(config.baseEnginePath) << std::endl;
        std::cerr << "        Make sure you compile the unmodified main branch into the 'build_base' folder." << std::endl;
        return 1;
    }

    // 3. Validate Opening Book
    if (!fileExists(config.openingBook)) {
        // Try to reconstruct from parts if they exist
        std::string partsCheck = "ls " + config.openingBook + ".part_* > /dev/null 2>&1";
        if (std::system(partsCheck.c_str()) == 0) {
            std::cout << "[INFO] Reconstructing opening book from parts..." << std::endl;
            std::string concatCmd = "cat " + config.openingBook + ".part_* > " + config.openingBook;
            std::system(concatCmd.c_str());
        }
    }

    if (!fileExists(config.openingBook)) {
        std::cerr << "[ERROR] Opening book not found at: " << fs::absolute(config.openingBook) << std::endl;
        return 1;
    }

    // 4. Construct Command Based on Selected Runner
    std::ostringstream cmd;
    
    if (config.useFastChess) {
        cmd << config.runnerPath
            << " -engine cmd=\"" << config.testEnginePath << "\" name=\"Test\""
            << " -engine cmd=\"" << config.baseEnginePath << "\" name=\"Base\""
            << " -each tc=\"" << config.timeControl << "\" option.Hash=16"
            << " -rounds " << config.games << " -games 2 -repeat"
            << " -concurrency " << config.concurrency
            << " -openings file=\"" << config.openingBook << "\" format=epd order=random plies=16"
            << " -sprt alpha=" << config.alpha << " beta=" << config.beta 
            << " elo0=" << config.elo0 << " elo1=" << config.elo1
            << " -recover"
            << " -ratinginterval 10";
    } else {
        cmd << config.runnerPath
            << " -engine cmd=\"" << config.testEnginePath << "\" name=\"Test\""
            << " -engine cmd=\"" << config.baseEnginePath << "\" name=\"Base\""
            << " -each proto=uci tc=\"" << config.timeControl << "\" option.Hash=16"
            << " -rounds " << config.games << " -games 2 -repeat"
            << " -concurrency " << config.concurrency
            << " -openings file=\"" << config.openingBook << "\" format=epd order=random"
            << " -sprt alpha=" << config.alpha << " beta=" << config.beta 
            << " elo0=" << config.elo0 << " elo1=" << config.elo1
            << " -recover"
            << " -ratinginterval 10";
    }

    std::cout << "[INFO] Runner: " << (config.useFastChess ? "FastChess" : "CuteChess-cli") << std::endl;
    std::cout << "[INFO] Starting Match: Test vs Base" << std::endl;
    std::cout << "[INFO] Time Control: " << config.timeControl << std::endl;
    std::cout << "[INFO] Concurrency: " << config.concurrency << " threads" << std::endl;
    std::cout << "[INFO] SPRT Bounds: [" << config.elo0 << ", " << config.elo1 << "]" << std::endl;
    std::cout << "===================================================" << std::endl;

    // 5. Execute
    int result = std::system(cmd.str().c_str());
    
    if (result != 0) {
        std::cerr << "[ERROR] SPRT match terminated with error code " << result << std::endl;
        return 1;
    }

    return 0;
}
