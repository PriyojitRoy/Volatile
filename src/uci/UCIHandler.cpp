#include "uci/UCIHandler.hpp"
#include "core/Bitboard.hpp"
#include "core/Constants.hpp"
#include "search/TT.hpp"
#include "search/hce/TimeManager.hpp"
#include "search/Tablebase.hpp"
#include "core/MoveGen.hpp"
#include "core/Move.hpp"
#include "core/Logo.hpp"

// Evaluation backend: USE_NNUE selects NNUE network, default is HCE
#ifdef USE_NNUE
#include "eval/nnue/Network.hpp"
#else
#include "eval/hce/Evaluate.hpp"
#endif

namespace VEngine {

    TranspositionTable g_tt(64);

    UCIHandler::UCIHandler() {
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
    }

    void UCIHandler::loop() {
        std::string line;
        std::string token;

        while (std::getline(std::cin, line)) {
            std::istringstream ss(line);
            ss >> token;

            if (token == "uci") {
                if (token == "uci") {
                std::cout << "id name Priyojit's Engine 2.0" << std::endl;
                std::cout << "id author Priyojit" << std::endl;
                
                // Existing Options
                std::cout << "option name SyzygyPath type string default <empty>" << std::endl;
                std::cout << "option name Hash type spin default 64 min 1 max 8192" << std::endl;

                std::cout << "uciok" << std::endl;
            }
            } 
            else if (token == "isready") {
                std::cout << "readyok" << std::endl;
            }
            else if (token == "setoption") {

                std::string nameToken, name, valueToken, value;
                ss >> nameToken >> name >> valueToken;
                std::getline(ss, value);
                value.erase(0, value.find_first_not_of(" \t"));

                while (!value.empty() && (value.back() == '\r' || value.back() == '\n' || value.back() == ' ' || value.back() == '"')) {
                    value.pop_back();
                }
                
                if (name == "SyzygyPath") {
                    Tablebase::init(value);
                }
                else if (name == "Hash") {
                    int mb = std::stoi(value);
                    g_tt.resize(mb);           
                    std::cout << "info string Hash successfully resized to " << mb << " MB" << std::endl;
                }
            }
            else if (token == "position") {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
                parsePosition(line); 
            }
            else if (token == "perft"){
                ss >> token;
                int nodes = runPerft(board,std::stoi(token));
                std::cout << "Nodes: " << nodes << std::endl;
            }
            else if (token == "go") {
                parseGo(line);             
            }
            else if (token == "stop") {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
            }
            else if (token == "ponderhit") {
                searcher.isPondering = false;
                searcher.startTime = std::chrono::high_resolution_clock::now();
            }
            else if (token == "ucinewgame") {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
                g_tt.clear();
                board.reset();
                board.parseFen(StartFen);
            }
            else if (token == "d") {
                board.print(); 
                std::cout << "Side to move: " << (board.sideToMove == White ? "White" : "Black") << std::endl;
                std::cout << "Key: " << std::hex << board.zorbitKey << std::dec << std::endl;
            }
            else if (token == "eval") {
#ifdef USE_NNUE
                int staticEval = Network::evaluate(board);
#else
                int staticEval = Evaluate::evaluate(board);
#endif
                std::cout << "info string Static Evaluation: " << staticEval << " cp" << std::endl;
            }
            else if (token == "play" || token == "playme") {
                playHuman();
            }
            else if (token == "quit") {
                searcher.stopSearch = true;
                if (searchThread.joinable()) searchThread.join();
                break;
            }
        }
    }

    void UCIHandler::playHuman() {
        std::cout << "--- Interactive Mode Started ---" << std::endl;
        std::cout << "Type moves in format 'e2e4'. Type 'exit' to quit mode." << std::endl;
        
        board.print();

        std::string input;
        while (true) {
            if (board.isThreefoldRepetition()) {
                std::cout << "Game Draw by 3-fold Repetition!" << std::endl;
                break;
            }
            if (board.halfMoveClock >= 100) {
                std::cout << "Game Draw by 50-move rule!" << std::endl;
                break;
            }
            if (board.isInsufficientMaterial()) {
                std::cout << "Game Draw by Insufficient Material!" << std::endl;
                break;
            }

            MoveList humanMoves;
            MoveGen::generateAllMoves(board, humanMoves);
            bool humanHasLegalMove = false;

            for (int i = 0; i < humanMoves.size(); i++) {
                if (board.makeMove(humanMoves[i])) {
                    board.unmakeMove(humanMoves[i]);
                    humanHasLegalMove = true;
                    break;
                }
            }

            if (!humanHasLegalMove) {
                if (board.isCheck()) std::cout << "Checkmate! Engine wins." << std::endl;
                else std::cout << "Stalemate! Draw." << std::endl;
                break;
            }

            std::cout << "\nYour move (" << (board.sideToMove == White ? "White" : "Black") << "): ";
            
            if (!(std::cin >> input)) break; 
            if (input == "exit" || input == "quit") break;

            Move userMove = parseMove(input);
            
            if (userMove.getData() == 0 || !board.makeMove(userMove)) {
                std::cout << "Invalid move! Try again." << std::endl;
                continue; 
            }

            board.print();

            MoveList engineMoves;
            MoveGen::generateAllMoves(board, engineMoves);
            bool engineHasLegalMove = false;
            
            for (int i = 0; i < engineMoves.size(); i++) {
                if (board.makeMove(engineMoves[i])) {
                    board.unmakeMove(engineMoves[i]);
                    engineHasLegalMove = true;
                    break;
                }
            }

            if (!engineHasLegalMove) {
                 if (board.isCheck()) std::cout << "Checkmate! You win!" << std::endl;
                 else std::cout << "Stalemate! Draw." << std::endl;
                 break;
            }

            std::cout << "Engine is thinking..." << std::endl;
            
            Move engineMove = searcher.getBestMove(board, 2000, 2000); 

            if (engineMove.getData() == 0) {
                std::cout << "Engine resigns (No moves found)." << std::endl;
                break;
            }

            std::cout << "Engine plays: " << engineMove.toString() << std::endl;
            board.makeMove(engineMove);
            board.print();
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

    void UCIHandler::parsePosition(const std::string& input) {
        std::istringstream ss(input);
        std::string token, type;
        ss >> token;

        if (!(ss >> type)) return;

        if (type == "startpos") {
            board.reset(); 
            board.parseFen(StartFen);
        } else if (type == "fen") {
            std::string fen;
            while (ss >> token && token != "moves") {
                fen += token + " ";
            }
            board.reset();
            board.parseFen(fen);
        }

        if (token == "moves" || (ss >> token && token == "moves")) {
            std::string moveStr;
            while (ss >> moveStr) {
                Move m = parseMove(moveStr);
                if (m.getData() != 0) { 
                    board.makeMove(m); 
                }
                else std::cout << "info string FATAL ERROR: Desync! Could not parse move: " << moveStr << std::endl;
            }
        }
    }

    void UCIHandler::parseGo(const std::string& input) {
        int wtime = 0, btime = 0, winc = 0, binc = 0;
        int movesToGo = 0, movetime = 0;
        bool infinite = false;
        bool ponder = false;

        std::istringstream ss(input);
        std::string token;
        ss >> token; 

        while (ss >> token) {
            if (token == "wtime") ss >> wtime;
            else if (token == "btime") ss >> btime;
            else if (token == "winc") ss >> winc;
            else if (token == "binc") ss >> binc;
            else if (token == "movestogo") ss >> movesToGo;
            else if (token == "movetime") ss >> movetime;
            else if (token == "infinite") infinite = true;
            else if (token == "ponder") ponder = true;
        }

        int optTime = 1000, maxTime = 1000; 

        if (movetime > 0) {
            optTime = maxTime = movetime; 
        } else if (wtime > 0 || btime > 0) {
            int currentPly = (board.fullMoveNumber - 1) * 2 + (board.sideToMove == Black ? 1 : 0);
            
            TimeAllocation alloc = TimeManager::calculateTime(wtime, btime, winc, binc, board.sideToMove, currentPly, movesToGo);
            optTime = alloc.optimumMs;
            maxTime = alloc.maximumMs;
        } else if (infinite) {
            optTime = maxTime = 2147483647; 
        }

        searcher.isPondering = ponder;

        if (searchThread.joinable()) searchThread.join();

        searchThread = std::thread([this, optTime, maxTime]() {
            Move bestMove = searcher.getBestMove(board, optTime, maxTime);
            std::cout << "bestmove " << bestMove.toString() << std::endl;
        });
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