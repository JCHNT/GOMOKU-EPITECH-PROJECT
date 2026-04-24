#pragma once

#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "protocol/parser.hpp"
#include <iosfwd>

namespace gomoku {

class Dispatcher {
public:
    Dispatcher(std::istream& in, std::ostream& out);

    void run();

private:
    void handle(const Parser::Command& cmd);
    void play_search_move(Color side);

    std::istream& in_;
    std::ostream& out_;
    Board board_;
    TranspositionTable tt_;
    TimeMgr tm_;
    bool board_mode_ = false;
    bool running_ = true;
};

} // namespace gomoku
