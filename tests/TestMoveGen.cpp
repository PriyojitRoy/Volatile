#include <iostream>
#include <string>
#include <chrono>
#include "chess/core/Board.hpp"
#include "chess/core/MoveGen.hpp"

using namespace ChessCore;

uint64_t perft(Board& board, int depth) {
    if (depth == 0) return 1ULL;
    MoveList list;
    MoveGen::generateAllMoves(board, list);
    uint64_t nodes = 0;
    for (int i = 0; i < list.size(); i++) {
        if (board.makeMove(list[i])) {
            nodes += perft(board, depth - 1);
            board.unmakeMove(list[i]);
        }
    }
    return nodes;
}

int main(int argc, char* argv[]) {
    Board board;
    std::string fen = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"; // Default
    int depth = 4; // Default

    if (argc >= 2) fen = argv[1];
    if (argc >= 3) depth = std::stoi(argv[2]);

    board.parseFen(fen);
    
    std::cout << "FEN: " << fen << std::endl;
    std::cout << "Testing MoveGen at Depth: " << depth << "..." << std::endl;

    auto start = std::chrono::high_resolution_clock::now();
    uint64_t result = perft(board, depth);
    auto end = std::chrono::high_resolution_clock::now();
    
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "Nodes found: " << result << std::endl;
    std::cout << "Time: " << duration << "ms" << std::endl;
    if (duration > 0) 
        std::cout << "NPS: " << (result * 1000 / duration) << std::endl;

    return 0;
}