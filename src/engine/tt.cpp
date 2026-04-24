#include "engine/tt.hpp"
#include <cstring>

namespace gomoku {

TranspositionTable::TranspositionTable(size_t num_entries) : table_(num_entries) {
    clear();
}

void TranspositionTable::clear() {
    std::memset(table_.data(), 0, table_.size() * sizeof(TTEntry));
    age_ = 0;
}

bool TranspositionTable::probe(uint64_t key, TTEntry& out) const {
    if (table_.empty()) return false;
    const size_t idx = key % table_.size();
    const TTEntry& e = table_[idx];
    if (e.key == key && key != 0) {
        out = e;
        return true;
    }
    return false;
}

void TranspositionTable::store(const TTEntry& e) {
    if (table_.empty()) return;
    const size_t idx = e.key % table_.size();
    TTEntry& slot = table_[idx];
    if (slot.key == 0 || slot.age != age_ || slot.depth <= e.depth) {
        slot = e;
        slot.age = age_;
    }
}

} // namespace gomoku
