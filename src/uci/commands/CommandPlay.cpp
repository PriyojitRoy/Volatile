#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include "core/MoveGen.hpp"
#include <iostream>

namespace VEngine {
    namespace Commands {
        void executePlay(UCIHandler& handler, std::istringstream& /*ss*/) {
            std::cout << "--- Interactive Mode Started ---" << std::endl;
            std::cout << "Type moves in format 'e2e4'. Type 'exit' to quit mode." << std::endl;
            
            handler.board.print();

            std::string input;
            while (true) {
                if (handler.board.isThreefoldRepetition()) {
                    std::cout << "Game Draw by 3-fold Repetition!" << std::endl;
                    break;
                }
                if (handler.board.halfMoveClock >= 100) {
                    std::cout << "Game Draw by 50-move rule!" << std::endl;
                    break;
                }
                if (handler.board.isInsufficientMaterial()) {
                    std::cout << "Game Draw by Insufficient Material!" << std::endl;
                    break;
                }

                MoveList humanMoves;
                MoveGen::generateAllMoves(handler.board, humanMoves);
                bool humanHasLegalMove = false;

                for (int i = 0; i < humanMoves.size(); i++) {
                    if (handler.board.makeMove(humanMoves[i])) {
                        handler.board.unmakeMove(humanMoves[i]);
                        humanHasLegalMove = true;
                        break;
                    }
                }

                if (!humanHasLegalMove) {
                    if (handler.board.isCheck()) std::cout << "Checkmate! Engine wins." << std::endl;
                    else std::cout << "Stalemate! Draw." << std::endl;
                    break;
                }

                std::cout << "\nYour move (" << (handler.board.sideToMove == White ? "White" : "Black") << "): ";
                
                if (!(std::cin >> input)) break; 
                if (input == "exit" || input == "quit") break;

                Move userMove = handler.parseMove(input);
                
                if (userMove.getData() == 0 || !handler.board.makeMove(userMove)) {
                    std::cout << "Invalid move! Try again." << std::endl;
                    continue; 
                }

                handler.board.print();

                MoveList engineMoves;
                MoveGen::generateAllMoves(handler.board, engineMoves);
                bool engineHasLegalMove = false;
                
                for (int i = 0; i < engineMoves.size(); i++) {
                    if (handler.board.makeMove(engineMoves[i])) {
                        handler.board.unmakeMove(engineMoves[i]);
                        engineHasLegalMove = true;
                        break;
                    }
                }

                if (!engineHasLegalMove) {
                     if (handler.board.isCheck()) std::cout << "Checkmate! You win!" << std::endl;
                     else std::cout << "Stalemate! Draw." << std::endl;
                     break;
                }

                std::cout << "Engine is thinking..." << std::endl;
                
                Move engineMove = handler.searcher.getBestMove(handler.board, 2000, 2000); 

                if (engineMove.getData() == 0) {
                    std::cout << "Engine resigns (No moves found)." << std::endl;
                    break;
                }

                std::cout << "Engine plays: " << engineMove.toString() << std::endl;
                handler.board.makeMove(engineMove);
                handler.board.print();
            }
        }
    }
}
