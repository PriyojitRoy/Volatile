#ifndef VENGINE_UCI_COMMANDS_HPP
#define VENGINE_UCI_COMMANDS_HPP

#include <sstream>

namespace VEngine {
    class UCIHandler;

    namespace Commands {
        void executeGo(UCIHandler& handler, std::istringstream& ss);
        void executePosition(UCIHandler& handler, std::istringstream& ss);
        void executePlay(UCIHandler& handler, std::istringstream& ss);
        void executeSetOption(UCIHandler& handler, std::istringstream& ss);
        void executePerft(UCIHandler& handler, std::istringstream& ss);
        void executeSysinfo(UCIHandler& handler, std::istringstream& ss);
        void executeDisplay(UCIHandler& handler, std::istringstream& ss);
    }
}

#endif // VENGINE_UCI_COMMANDS_HPP
