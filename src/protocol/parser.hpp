#pragma once

#include <string>

namespace gomoku {

class Parser {
public:
    enum Kind {
        START,
        TURN,
        BEGIN,
        BOARD_START,
        BOARD_CELL,
        DONE,
        INFO,
        END,
        ABOUT,
        RESTART,
        UNKNOWN,
        MALFORMED,
    };

    struct Command {
        Kind kind = UNKNOWN;
        int size = 0;
        int x = 0, y = 0, who = 0;
        std::string info_key;
        std::string info_value;
        std::string raw;
    };

    static Command parse(std::string line);
};

} // namespace gomoku
