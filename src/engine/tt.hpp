#pragma once

#include "util/types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

enum TTFlag : uint8_t {
    TT_EXACT = 0,
    TT_LOWER = 1,
    TT_UPPER = 2,
};

struct TTEntry {
    uint64_t key;
    int16_t depth;
    int32_t score;
    uint8_t flag;
    Move best;
    uint16_t age;
};

class TranspositionTable {
public:
    explicit TranspositionTable(size_t num_entries);

    bool probe(uint64_t key, TTEntry& out) const;
    void store(const TTEntry& e);
    void clear();
    void new_search() { ++age_; }

private:
    std::vector<TTEntry> table_;
    uint16_t age_ = 0;
};

} // namespace gomoku
