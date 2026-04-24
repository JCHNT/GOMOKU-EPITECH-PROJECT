#include "parallel.hpp"
#include "engine/search.hpp"
#include <algorithm>
#include <atomic>
#include <thread>
#include <vector>

namespace gomoku {

ParallelSearch::ParallelSearch(TranspositionTable& tt, TimeMgr& tm, int threads)
    : tt_(tt), tm_(tm), threads_(std::max(1, threads)) {}

Move ParallelSearch::go(Board& root, Color side, int max_depth) {
    std::atomic<bool> helpers_stop{false};
    std::vector<std::thread> helpers;
    helpers.reserve(threads_ - 1);

    for (int t = 1; t < threads_; ++t) {
        helpers.emplace_back([&, t]() {
            Board b = root;
            Search s(tt_, tm_);
            while (!helpers_stop.load() && !tm_.should_stop()) {
                int d = 1 + (t % std::max(1, max_depth));
                (void) s.go(b, side, d);
            }
        });
    }

    Search main(tt_, tm_);
    Move best = main.go(root, side, max_depth);

    helpers_stop.store(true);
    for (auto& h : helpers) h.join();
    return best;
}

} // namespace gomoku
