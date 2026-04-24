#include "book.hpp"
#include "board/board.hpp"
#include <fstream>
#include <sstream>

namespace gomoku {

Move Book::transform(Move m, int sym) {
    int x = m.x, y = m.y;
    const int N = BOARD_SIZE - 1;
    if (sym & 4) x = N - x;
    switch (sym & 3) {
        case 0: return {(int8_t)x, (int8_t)y};
        case 1: return {(int8_t)(N - y), (int8_t)x};
        case 2: return {(int8_t)(N - x), (int8_t)(N - y)};
        case 3: return {(int8_t)y, (int8_t)(N - x)};
    }
    return m;
}

int Book::inverse_sym(int sym) {
    static const int inv[8] = {0, 3, 2, 1, 4, 5, 6, 7};
    return inv[sym];
}

bool Book::load_file(const std::string& path) {
    std::ifstream f(path);
    if (!f) return false;
    std::stringstream buf; buf << f.rdbuf();
    load_string(buf.str());
    return true;
}

static Move parse_move(const std::string& tok) {
    size_t c = tok.find(',');
    if (c == std::string::npos) return NO_MOVE;
    try {
        return {(int8_t)std::stoi(tok.substr(0, c)), (int8_t)std::stoi(tok.substr(c + 1))};
    } catch (...) { return NO_MOVE; }
}

void Book::load_string(const std::string& content) {
    std::stringstream ss(content);
    std::string line;
    while (std::getline(ss, line)) {
        std::stringstream ls(line);
        std::vector<Move> moves;
        std::string tok;
        while (ls >> tok) {
            Move m = parse_move(tok);
            if (Board::in_bounds(m)) moves.push_back(m);
        }
        if (moves.empty()) continue;
        Entry e;
        e.reply = moves.back();
        moves.pop_back();
        e.history = std::move(moves);
        entries_.push_back(std::move(e));
    }
}

bool Book::lookup(const std::vector<Move>& history, Move& out) const {
    for (int sym = 0; sym < 8; ++sym) {
        std::vector<Move> h_t;
        h_t.reserve(history.size());
        for (auto m : history) h_t.push_back(transform(m, sym));
        for (const auto& e : entries_) {
            if (e.history == h_t) {
                out = transform(e.reply, inverse_sym(sym));
                return true;
            }
        }
    }
    return false;
}

} // namespace gomoku
