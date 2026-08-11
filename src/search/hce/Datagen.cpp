#include "chess/search/hce/Datagen.hpp"
#include "chess/search/hce/Search.hpp"
#include "chess/core/MoveGen.hpp"
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>
#include <mutex>

namespace ChessCore {


    std::vector<std::string> Datagen::loadEPD(const std::string& filename) {
        std::vector<std::string> lines;
        std::ifstream file(filename);
        std::string line;
        while (std::getline(file, line)) {
            if (!line.empty()) lines.push_back(line);
        }
        return lines;
    }

    int Datagen::checkGameOver(Board& board) {
        MoveList ml;
        MoveGen::generateAllMoves(board, ml);
        int legalMoves = 0;
        
        for (int i = 0; i < ml.size(); i++) {
            if (board.makeMove(ml[i])) {
                legalMoves++;
                board.unmakeMove(ml[i]);
            }
        }

        if (legalMoves == 0) {
            if (board.isCheck()) return (board.sideToMove == White) ? -1 : 1; 
            return 0; 
        }
        if (board.halfMoveClock >= 100 || board.isThreefoldRepetition()) return 0; 
        return -2; 
    }

    uint8_t Datagen::getPieceAtSquare(const Board& board, int sq) {
        uint64_t sqMask = 1ULL << sq;
        if (!(board.occupancy[Both] & sqMask)) return 0; 
        
        for (int p = Pawn; p <= King; p++) {
            if (board.getPieces(p, White) & sqMask) return p + 1;       
            if (board.getPieces(p, Black) & sqMask) return (p + 1) + 6; 
        }
        return 0;
    }

    PackedEntry Datagen::compressPosition(const TrainingEntry& entry) {
        PackedEntry packed;
        packed.score = static_cast<int16_t>(entry.score);
        packed.result = static_cast<int8_t>(entry.result);
        packed.flags = (entry.board.sideToMove == White) ? 1 : 0;
        
        for (int i = 0; i < 32; i++) {
            int sq1 = i * 2;
            int sq2 = i * 2 + 1;
            
            uint8_t p1 = getPieceAtSquare(entry.board, sq1) & 0x0F;
            uint8_t p2 = getPieceAtSquare(entry.board, sq2) & 0x0F;
            
            packed.squares[i] = p1 | (p2 << 4);
        }
        return packed;
    }

    void Datagen::writeToBinaryFile(const std::vector<TrainingEntry>& gamePositions, const std::string& outputFile) {
        std::ofstream outFile(outputFile, std::ios::app | std::ios::binary);
        if (!outFile) return;

        for (const auto& entry : gamePositions) {
            PackedEntry packed = compressPosition(entry);
            outFile.write(reinterpret_cast<const char*>(&packed), sizeof(PackedEntry));
        }
        outFile.close();
    }



    //extern void updateLMRTable(double c1, double c2);

    void Datagen::applyParams(const TunableParams& /*p*/) {
        //updateLMRTable(p.c1, p.c2);
    }

    double Datagen::playMatch(Board board, const TunableParams& whiteParams, const TunableParams& blackParams) {
        int gameResult = -2;
        Search searcher;
        int moves = 0;

        while (gameResult == -2 && moves < 200) { 
            if (board.sideToMove == White) applyParams(whiteParams);
            else applyParams(blackParams);

            Move bestMove = searcher.getBestMove(board, 50, 50); 
            
            if (bestMove.getData() == 0) break; 

            board.makeMove(bestMove);
            gameResult = checkGameOver(board);
            moves++;
        }
        
        if (gameResult == -2) return 0.5;
        if (gameResult == 1) return 1.0;       
        else if (gameResult == -1) return 0.0; 
        return 0.5;                            
    }

    void Datagen::runSPSATuner(const std::string& epdFile) {
        std::vector<std::string> epdLines = loadEPD(epdFile);
        if (epdLines.empty()) {
            std::cout << "Error: Could not load EPD file for tuning!" << std::endl;
            return;
        }

        std::cout << "--- Starting LMR Formula SPSA Tuner ---" << std::endl;

        int startIter = 1;
        
        TunableParams current = {0.75, 2.25}; 
        
        std::ifstream inFile("spsa_state.txt");
        if (inFile) {
            inFile >> startIter >> current.c1 >> current.c2;
            std::cout << "Found save file! Resuming from Iteration " << startIter << std::endl;
        }

        const double A = 10.0;
        const double a_lr = 0.05;   
        const double c_step = 0.05; 
        const double alpha = 0.602;
        const double gamma = 0.101;
        
        const double scale_c1 = 1.0;  
        const double scale_c2 = 2.0;  

        const int TOTAL_GAMES_PER_ITER = 20;
        const int NUM_THREADS = 4;
        const int GAMES_PER_THREAD = TOTAL_GAMES_PER_ITER / NUM_THREADS;

        std::mt19937 master_rng(12345 + startIter); 
        std::bernoulli_distribution coinFlip(0.5);

        for (int iter = startIter; iter <= 1000; iter++) {
            double ak = a_lr / std::pow(iter + A, alpha);
            double ck = c_step / std::pow(iter, gamma);

            double d_c1 = coinFlip(master_rng) ? 1.0 : -1.0;
            double d_c2 = coinFlip(master_rng) ? 1.0 : -1.0;

            TunableParams plus = {
                current.c1 + ck * d_c1 * scale_c1,
                current.c2 + ck * d_c2 * scale_c2
            };
            
            TunableParams minus = {
                current.c1 - ck * d_c1 * scale_c1,
                current.c2 - ck * d_c2 * scale_c2
            };

            std::mutex scoreMutex;
            double totalPlusScore = 0;
            std::vector<std::thread> workers;

            for (int t = 0; t < NUM_THREADS; t++) {
                workers.emplace_back([&, t]() {
                    double localPlusScore = 0;
                    std::mt19937 local_rng(12345 + iter * 100 + t);
                    std::uniform_int_distribution<int> local_dist(0, epdLines.size() - 1);

                    for (int g = 0; g < GAMES_PER_THREAD; g++) {
                        int globalGameIdx = t * GAMES_PER_THREAD + g;
                        Board board;
                        board.parseFen(epdLines[local_dist(local_rng)]);

                        if (globalGameIdx % 2 == 0) {
                            localPlusScore += playMatch(board, plus, minus);
                        } else {
                            localPlusScore += (1.0 - playMatch(board, minus, plus)); 
                        }
                        std::cout << "." << std::flush;
                    }

                    std::lock_guard<std::mutex> lock(scoreMutex);
                    totalPlusScore += localPlusScore;
                });
            }

            for (auto& worker : workers) {
                worker.join();
            }

            double winRate = totalPlusScore / TOTAL_GAMES_PER_ITER; 
            double gradient = (winRate - 0.5) / 0.5; 

            current.c1 += ak * gradient * d_c1 * scale_c1;
            current.c2 += ak * gradient * d_c2 * scale_c2;

            std::cout << "\nIteration " << iter << " | Engine Plus WinRate: " << (winRate * 100) << "%" << std::endl;
            std::cout << "  C1 (Base): " << current.c1 
                      << " | C2 (Divisor): " << current.c2 << "\n\n";

            std::ofstream outFile("spsa_state.txt");
            if (outFile) {
                outFile << (iter + 1) << " " << current.c1 << " " << current.c2 << "\n";
            }
        }
    }

    struct StockfishProcess {
        int pipe_in[2];
        int pipe_out[2];
        pid_t pid;
        FILE* out_stream;

        bool start(const std::string& path = "stockfish") {
            if (pipe(pipe_in) < 0 || pipe(pipe_out) < 0) return false;
            pid = fork();
            if (pid == 0) {
                // Child process: Redirect stdin/stdout to the pipes, then launch Stockfish
                dup2(pipe_in[0], STDIN_FILENO);
                dup2(pipe_out[1], STDOUT_FILENO);
                close(pipe_in[1]); close(pipe_out[0]);
                execlp(path.c_str(), path.c_str(), nullptr);
                exit(1); 
            }
            // Parent process
            close(pipe_in[0]); close(pipe_out[1]);
            out_stream = fdopen(pipe_out[0], "r");
            
            // Initialize UCI parameters
            send("uci");
            while (readLine() != "uciok");
            send("setoption name Threads value 1");
            send("setoption name Hash value 16"); // Keep memory footprint small per thread
            send("isready");
            while (readLine() != "readyok");
            return true;
        }

        void send(const std::string& cmd) {
            std::string full = cmd + "\n";
            if(write(pipe_in[1], full.c_str(), full.length())==-1){}
        }

        std::string readLine() {
            char buf[2048];
            if (fgets(buf, sizeof(buf), out_stream)) {
                std::string s(buf);
                if (!s.empty() && s.back() == '\n') s.pop_back();
                if (!s.empty() && s.back() == '\r') s.pop_back();
                return s;
            }
            return "";
        }

        void stop() {
            send("quit");
            fclose(out_stream);
            close(pipe_in[1]);
            waitpid(pid, nullptr, 0);
        }
    };


    int parseStockfishEval(const std::string& infoLine, int sideToMove) {
        std::istringstream iss(infoLine);
        std::string token;
        int score = 0;
        
        while (iss >> token) {
            if (token == "score") {
                iss >> token;
                if (token == "cp") {
                    iss >> score;
                } else if (token == "mate") {
                    int mateIn;
                    iss >> mateIn;
                    score = (mateIn > 0) ? 32000 : -32000;
                }
                if (sideToMove == Black) score = -score;
                return score;
            }
        }
        return 0;
    }


    void Datagen::generateData(int gamesToPlay, const std::string& epdFile, const std::string& outputFile) {
        std::vector<std::string> epdLines = loadEPD(epdFile);
        if (epdLines.empty()) {
            std::cout << "Error: Could not load EPD file!" << std::endl;
            return;
        }

        std::mt19937 rng(std::random_device{}());
        std::uniform_int_distribution<int> dist(0, epdLines.size() - 1);

        StockfishProcess sf;
        if (!sf.start("stockfish")) {
            std::cout << "Error: Could not start Stockfish binary." << std::endl;
            return;
        }

        int gamesCompleted = 0;
        std::vector<TrainingEntry> gamePositions; 

        while (gamesCompleted < gamesToPlay) {
            Board board; 
            std::string startFen = epdLines[dist(rng)];
            board.parseFen(startFen); 
            
            MoveList ml;
            MoveGen::generateAllMoves(board, ml);
            if (ml.size() > 0) {
                std::uniform_int_distribution<int> moveDist(0, ml.size() - 1);
                board.makeMove(ml[moveDist(rng)]);
            }

            std::vector<TrainingEntry> tempPositions;
            int gameResult = -2;
            std::string moveHistory = ""; 
            int totalPlies = 0;

            while (gameResult == -2 && board.halfMoveClock < 100 && totalPlies < 400) {
                sf.send("position fen " + startFen + " moves " + moveHistory);
                
                sf.send("go nodes 5000"); 
                
                std::string bestMoveUci = "";
                int currentScore = 0;
                bool scoreFound = false;

                while (true) {
                    std::string line = sf.readLine();
                    if (line.find("info depth") != std::string::npos && line.find("score") != std::string::npos) {
                        currentScore = parseStockfishEval(line, board.sideToMove);
                        scoreFound = true;
                    }
                    if (line.find("bestmove") == 0) {
                        bestMoveUci = line.substr(9, 4); 
                        if (line.length() > 13) bestMoveUci += line[13]; 
                        break;
                    }
                }

                if (bestMoveUci.empty() || bestMoveUci == "(non") break;

                if (scoreFound && !board.isCheck()) {
                    TrainingEntry entry;
                    entry.board = board;
                    entry.score = currentScore;
                    tempPositions.push_back(entry);
                }

                int sfFrom = (bestMoveUci[0] - 'a') + (bestMoveUci[1] - '1') * 8;
                int sfTo   = (bestMoveUci[2] - 'a') + (bestMoveUci[3] - '1') * 8;

                MoveGen::generateAllMoves(board, ml);
                bool moveFound = false;
                
                for (int i = 0; i < ml.size(); i++) {
                    if (ml[i].getFrom() == sfFrom && ml[i].getTo() == sfTo) { 
                        
                        if (ml[i].isCapture() && !tempPositions.empty()) {
                            tempPositions.pop_back();
                        }

                        board.makeMove(ml[i]);
                        moveFound = true;
                        break;
                    }
                }
                
                if (!moveFound) break; 
                moveHistory += bestMoveUci + " ";
                gameResult = checkGameOver(board); 
                totalPlies++;
            }

            gamePositions.clear();
            for (size_t i = 0; i < tempPositions.size(); i += 8) {
                tempPositions[i].result = gameResult;
                gamePositions.push_back(tempPositions[i]);
            }
            
            if (!gamePositions.empty()) {
                writeToBinaryFile(gamePositions, outputFile);
            }
            
            gamesCompleted++;
            if (gamesCompleted % 500 == 0) {
                std::cout << "Thread Output [" << outputFile << "] - Completed: " << gamesCompleted << " games." << std::endl;
            }
        }
        sf.stop();
    }

}