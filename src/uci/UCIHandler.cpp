#include "uci/UCIHandler.hpp"
#include "core/Bitboard.hpp"
#include "core/Constants.hpp"
#include "search/TT.hpp"
#include "search/TimeManager.hpp"
#include "search/Tablebase.hpp"
#include "core/MoveGen.hpp"
#include "core/Move.hpp"
#include "core/Logo.hpp"
#include "uci/commands/Commands.hpp"
#include <iostream>
#include <cstdlib>

// Evaluation backend: USE_NNUE selects NNUE network, default is HCE
#ifdef USE_NNUE
#include "eval/nnue/Network.hpp"
#else
#include "eval/hce/Evaluate.hpp"
#endif

namespace VEngine {

    TranspositionTable g_tt(64);

    UCIHandler::UCIHandler() : isRunning(true) {
        Bitboard::init();
        board.initZobrist();
        
#ifdef USE_NNUE
        if(!Network::load("data/network-20220625.nnue")) {
        }
#else
        // HCE does not require loading a network file
#endif
        
        board.parseFen(StartFen);

        std::cout << VEngine::Constants::ENGINE_LOGO << std::endl;
        std::cout << "\033[1;36mVolatile Chess Engine By Priyojit Roy\033[0m" << std::endl;
        std::cout << "Visit the engine at: " << VEngine::Constants::ENGINE_GITHUB_LINK << std::endl;
        
        registerCommands();
    }

    void UCIHandler::registerCommands() {
        commands["uci"] = {
            "uci", 
            "Tell engine to use the uci (universal chess interface)", 
            [](std::istringstream&) {
                std::cout << "id name Volatile Chess Engine VCE" << std::endl;
#ifdef USE_NNUE
                std::cout << "info string WARNING: Engine compiled with USE_NNUE=ON but NNUE is currently a stub! Evaluation will be incorrect/invalid." << std::endl;
#endif
                std::cout << "option name SyzygyPath type string default <empty>" << std::endl;
                std::cout << "option name BookPath type string default <empty>" << std::endl;
                std::cout << "option name Hash type spin default 64 min 1 max 8192" << std::endl;
                std::cout << "uciok" << std::endl;
            }
        };

        commands["isready"] = {
            "isready", 
            "Checks if the engine is ready", 
            [](std::istringstream&) { std::cout << "readyok" << std::endl; }
        };

        commands["setoption"] = {
            "setoption name <name> [value <value>]", 
            "Change engine parameters", 
            [this](std::istringstream& ss) { Commands::executeSetOption(*this, ss); }
        };

        commands["position"] = {
            "position [fen <fenstring> | startpos] moves <move1> ...", 
            "Set up the position on the board", 
            [this](std::istringstream& ss) { Commands::executePosition(*this, ss); }
        };

        commands["perft"] = {
            "perft <depth>", 
            "Run performance test", 
            [this](std::istringstream& ss) { Commands::executePerft(*this, ss); }
        };

        commands["go"] = {
            "go [wtime <x>] [btime <y>] [movetime <z>] ...", 
            "Start calculating the best move", 
            [this](std::istringstream& ss) { Commands::executeGo(*this, ss); }
        };

        commands["stop"] = {
            "stop", 
            "Stop calculating as soon as possible", 
            [this](std::istringstream&) {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
            }
        };

        commands["ponderhit"] = {
            "ponderhit", 
            "User played the expected move, engine can switch from ponder to normal search", 
            [this](std::istringstream&) {
                searcher.isPondering = false;
                searcher.startTime = std::chrono::high_resolution_clock::now();
            }
        };

        commands["ucinewgame"] = {
            "ucinewgame", 
            "Clear hash tables and reset board for a new game", 
            [this](std::istringstream&) {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
                g_tt.clear();
                board.reset();
                board.parseFen(StartFen);
            }
        };

        commands["d"] = {
            "d", 
            "Display the current board in ASCII and detailed state info", 
            [this](std::istringstream& ss) { Commands::executeDisplay(*this, ss); }
        };

        commands["sysinfo"] = {
            "sysinfo", 
            "Show compiler, architecture, and instruction set information", 
            [this](std::istringstream& ss) { Commands::executeSysinfo(*this, ss); }
        };
        commands["compiler"] = commands["sysinfo"];

        commands["eval"] = {
            "eval", 
            "Show static evaluation of current position", 
            [this](std::istringstream&) {
#ifdef USE_NNUE
                int staticEval = Network::evaluate(board);
#else
                int staticEval = Evaluate::evaluate(board);
#endif
                std::cout << "info string Static Evaluation: " << staticEval << " cp" << std::endl;
            }
        };

        commands["play"] = {
            "play", 
            "Play against the engine in terminal", 
            [this](std::istringstream& ss) { Commands::executePlay(*this, ss); }
        };
        commands["playme"] = commands["play"]; 

        commands["help"] = {
            "help", 
            "List all available commands and their syntax", 
            [this](std::istringstream&) {
                std::cout << "\nAvailable Commands:\n";
                std::cout << "--------------------------------------------------------\n";
                for (const auto& pair : commands) {
                    std::cout << "\033[1;33m" << pair.first << "\033[0m\n";
                    std::cout << "  Syntax: " << pair.second.syntax << "\n";
                    std::cout << "  Desc:   " << pair.second.description << "\n\n";
                }
                std::cout << "--------------------------------------------------------\n";
            }
        };

        commands["quit"] = {
            "quit", 
            "Exit the engine", 
            [this](std::istringstream&) {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
                isRunning = false;
            }
        };
    }

    void UCIHandler::loop() {
        std::string line;
        std::string token;

        while (isRunning && std::getline(std::cin, line)) {
            std::istringstream ss(line);
            token.clear();
            ss >> token;

            if (token.empty()) continue;

            auto it = commands.find(token);
            if (it != commands.end()) {
                it->second.handler(ss);
            } else if (!token.empty()) {
                std::cout << "Unknown command: '" << token << "'. Type 'help' for a list of commands." << std::endl;
            }
        }
    }

    Move UCIHandler::parseMove(const std::string& moveStr) {
        MoveList moves;
        MoveGen::generateAllMoves(board, moves);

        for (int i = 0; i < moves.size(); i++) {
            Move m = moves[i];
            if (m.toString() == moveStr) {
                return m;
            }
        }
        return Move();
    }

    uint64_t UCIHandler::runPerft(Board& b, int depth) {
        if (depth == 0) return 1ULL;

        MoveList moves;
        MoveGen::generateAllMoves(b, moves);
        
        uint64_t nodes = 0;
        for (int i = 0; i < moves.size(); i++) {
            if (b.makeMove(moves[i])) {
                nodes += runPerft(b, depth - 1);
                b.unmakeMove(moves[i]);
            }
        }
        return nodes;
    }
}