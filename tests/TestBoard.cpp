#include <iostream>
#include <string>
#include "chess/core/Board.hpp"

using namespace ChessCore;

int main(int argc, char* argv[]) {
    Board board;
    std::string fen = (argc >= 2) ? argv[1] : "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    board.parseFen(fen);
    
    std::cout << "Parsed FEN Board State:" << std::endl;
    board.print();
    return 0;
}