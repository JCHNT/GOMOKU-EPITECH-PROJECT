#pragma once

#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "protocol/parser.hpp"
#include <iosfwd>
#include <vector>

#ifdef BONUS
#include "book.hpp"
#endif

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

#ifdef BONUS
    Book book_;
    std::vector<Move> book_history_;
#endif
};

} // namespace gomoku
