#include "uci/commands/Commands.hpp"
#include "uci/UCIHandler.hpp"
#include "search/Tablebase.hpp"
#include "search/OpeningBook.hpp"
#include "search/TT.hpp"
#include <iostream>

namespace VEngine {
    extern TranspositionTable g_tt;

    namespace Commands {
        void executeSetOption(UCIHandler& /*handler*/, std::istringstream& ss) {
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
            else if (name == "BookPath") {
                OpeningBook::init(value);
            }
            else if (name == "Hash") {
                int mb = std::stoi(value);
                g_tt.resize(mb);           
                std::cout << "info string Hash successfully resized to " << mb << " MB" << std::endl;
            }
        }
    }
}
