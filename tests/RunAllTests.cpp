#include <iostream>
#include <cstdlib>

int main() {
    std::cout << "========================================" << std::endl;
    std::cout << " Running All Engine Tests" << std::endl;
    std::cout << "========================================" << std::endl;
    
    std::cout << "\n--- 1. Move Generation Tests ---" << std::endl;
    if (std::system("./run_movegen") != 0) {
        std::cerr << "Warning: Move Generation tests failed or threw an error." << std::endl;
    }
    
    std::cout << "\n--- 2. Evaluation Position Tests ---" << std::endl;
    if (std::system("./run_eval") != 0) {
        std::cerr << "Warning: Evaluation tests failed or threw an error." << std::endl;
    }
    
    std::cout << "\n--- 3. SPRT Match Test ---" << std::endl;
    if (std::system("./run_sprt") != 0) {
        std::cerr << "Warning: SPRT tests failed or threw an error." << std::endl;
    }
    
    std::cout << "========================================" << std::endl;
    std::cout << " All tests executed." << std::endl;
    std::cout << "========================================" << std::endl;
    return 0;
}
