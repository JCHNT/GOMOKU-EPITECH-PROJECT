#include "protocol/dispatcher.hpp"
#include <iostream>
#include <sstream>
#include <string>

#ifdef BONUS
#include "tss.hpp"
#include "parallel.hpp"
#include <thread>
#endif

namespace gomoku {

static constexpr size_t TT_ENTRIES = 1u << 20;

Dispatcher::Dispatcher(std::istream& in, std::ostream& out)
    : in_(in), out_(out), tt_(TT_ENTRIES) {
#ifdef BONUS
    book_.load_file("data/book.txt");
#endif
}

void Dispatcher::play_search_move(Color side) {
    int empty = BOARD_CELLS - board_.stone_count();
    tm_.start_turn(empty);

#ifdef BONUS
    Move book_m;
    if (book_.lookup(book_history_, book_m)
        && Board::in_bounds(book_m)
        && board_.at(book_m.x, book_m.y) == EMPTY) {
        board_.make_move(book_m, side);
        book_history_.push_back(book_m);
        out_ << static_cast<int>(book_m.x) << "," << static_cast<int>(book_m.y) << "\n";
        out_.flush();
        return;
    }
    Move tss_move;
    if (TSS::find_vcf(board_, side, 10, tss_move)) {
        board_.make_move(tss_move, side);
        book_history_.push_back(tss_move);
        out_ << static_cast<int>(tss_move.x) << "," << static_cast<int>(tss_move.y) << "\n";
        out_.flush();
        return;
    }
    if (TSS::find_vct(board_, side, 8, tss_move)) {
        board_.make_move(tss_move, side);
        book_history_.push_back(tss_move);
        out_ << static_cast<int>(tss_move.x) << "," << static_cast<int>(tss_move.y) << "\n";
        out_.flush();
        return;
    }
#endif

#ifdef BONUS
    int nthreads = static_cast<int>(std::thread::hardware_concurrency());
    if (nthreads < 1) nthreads = 1;
    if (nthreads > 8) nthreads = 8;
    ParallelSearch ps(tt_, tm_, nthreads);
    Move m = ps.go(board_, side, 64);
#else
    Search s(tt_, tm_);
    Move m = s.go(board_, side, 64);
#endif
    if (!Board::in_bounds(m)) {
        for (int y = 0; y < BOARD_SIZE && !Board::in_bounds(m); ++y)
            for (int x = 0; x < BOARD_SIZE; ++x)
                if (board_.at(x, y) == EMPTY) { m = {(int8_t)x, (int8_t)y}; break; }
    }
    board_.make_move(m, side);
#ifdef BONUS
    book_history_.push_back(m);
#endif
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
#ifdef BONUS
            book_history_.push_back({(int8_t)c.x, (int8_t)c.y});
#endif
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
