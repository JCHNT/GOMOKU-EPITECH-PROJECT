#pragma once

#include "util/types.hpp"
#include <string>
#include <vector>

namespace gomoku {

class Book {
public:
    bool load_file(const std::string& path);
    void load_string(const std::string& content);
    bool lookup(const std::vector<Move>& history, Move& out) const;

private:
    struct Entry {
        std::vector<Move> history;
        Move reply;
    };
    std::vector<Entry> entries_;

    static Move transform(Move m, int sym);
    static int inverse_sym(int sym);
};

} // namespace gomoku
