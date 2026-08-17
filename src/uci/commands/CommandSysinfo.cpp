#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include <iostream>

namespace VEngine {
    namespace Commands {
        void executeSysinfo(UCIHandler& /*handler*/, std::istringstream& /*ss*/) {
            std::cout << "info string Compiler: " << __VERSION__ << std::endl;
            std::cout << "info string Architecture: " << (sizeof(void*) == 8 ? "64-bit" : "32-bit") << std::endl;
#ifdef __AVX2__
            std::cout << "info string AVX2: Yes" << std::endl;
#else
            std::cout << "info string AVX2: No" << std::endl;
#endif
#ifdef __BMI2__
            std::cout << "info string BMI2: Yes" << std::endl;
#else
            std::cout << "info string BMI2: No" << std::endl;
#endif
#ifdef __POPCNT__
            std::cout << "info string POPCNT: Yes" << std::endl;
#else
            std::cout << "info string POPCNT: No" << std::endl;
#endif
#ifdef USE_NNUE
            std::cout << "info string Eval: NNUE" << std::endl;
#else
            std::cout << "info string Eval: HCE" << std::endl;
#endif
        }
    }
}
