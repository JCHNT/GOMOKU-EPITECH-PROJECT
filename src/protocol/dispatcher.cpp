#include "protocol/dispatcher.hpp"
#include <iostream>
#include <sstream>
#include <string>

namespace gomoku {

static constexpr size_t TT_ENTRIES = 1u << 20;

Dispatcher::Dispatcher(std::istream& in, std::ostream& out)
    : in_(in), out_(out), tt_(TT_ENTRIES) {}

void Dispatcher::play_search_move(Color side) {
    int empty = BOARD_CELLS - board_.stone_count();
    tm_.start_turn(empty);
    Search s(tt_, tm_);
    Move m = s.go(board_, side, 64);
    if (!Board::in_bounds(m)) {
        for (int y = 0; y < BOARD_SIZE && !Board::in_bounds(m); ++y)
            for (int x = 0; x < BOARD_SIZE; ++x)
                if (board_.at(x, y) == EMPTY) { m = {(int8_t)x, (int8_t)y}; break; }
    }
    board_.make_move(m, side);
    out_ << static_cast<int>(m.x) << "," << static_cast<int>(m.y) << "\n";
    out_.flush();
}

void Dispatcher::handle(const Parser::Command& c) {
    using K = Parser::Kind;
    switch (c.kind) {
        case K::START:
            if (c.size == BOARD_SIZE) { board_ = Board(); out_ << "OK\n"; }
            else out_ << "ERROR size not supported, only 20 accepted\n";
            out_.flush();
            break;
        case K::TURN:
            board_.make_move({(int8_t)c.x, (int8_t)c.y}, WHITE);
            play_search_move(BLACK);
            break;
        case K::BEGIN:
            play_search_move(BLACK);
            break;
        case K::BOARD_START:
            board_ = Board();
            board_mode_ = true;
            break;
        case K::BOARD_CELL:
            if (board_mode_) {
                Color c2 = (c.who == 1) ? BLACK : WHITE;
                board_.make_move({(int8_t)c.x, (int8_t)c.y}, c2);
            }
            break;
        case K::DONE:
            board_mode_ = false;
            play_search_move(BLACK);
            break;
        case K::INFO:
            tm_.set_info(c.info_key, c.info_value);
            break;
        case K::END:
            running_ = false;
            break;
        case K::ABOUT:
            out_ << "name=\"GOMOKU-EPITECH\", version=\"1.0\", author=\"Epitech\", country=\"FR\"\n";
            out_.flush();
            break;
        case K::RESTART:
            board_ = Board();
            out_ << "OK\n";
            out_.flush();
            break;
        case K::MALFORMED:
            out_ << "ERROR malformed command\n";
            out_.flush();
            break;
        case K::UNKNOWN:
        default:
            out_ << "UNKNOWN " << c.raw << "\n";
            out_.flush();
            break;
    }
}

void Dispatcher::run() {
    std::string line;
    while (running_ && std::getline(in_, line)) {
        auto cmd = Parser::parse(line);
        handle(cmd);
    }
}

} // namespace gomoku
