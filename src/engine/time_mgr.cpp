#include "engine/time_mgr.hpp"
#include <algorithm>
#include <climits>

namespace gomoku {

static int parse_int(const std::string& s, int fallback) {
    try { return std::stoi(s); } catch (...) { return fallback; }
}

void TimeMgr::set_info(const std::string& key, const std::string& value) {
    if (key == "timeout_turn")       timeout_turn_ms_ = parse_int(value, timeout_turn_ms_);
    else if (key == "timeout_match") timeout_match_ms_ = parse_int(value, timeout_match_ms_);
    else if (key == "time_left")     time_left_ms_ = parse_int(value, time_left_ms_);
    // unknown keys silently ignored
}

void TimeMgr::start_turn(int empty_cells) {
    turn_start_ = std::chrono::steady_clock::now();
    stopped_.store(false);

    int by_turn = (timeout_turn_ms_ > 0)
        ? static_cast<int>(timeout_turn_ms_ * 0.9)
        : 4500;

    int by_match = INT_MAX;
    if (time_left_ms_ > 0 && empty_cells > 0) {
        int denom = std::max(20, empty_cells);
        by_match = time_left_ms_ / denom;
    }
    budget_ms_ = std::max(50, std::min(by_turn, by_match));
}

static int elapsed_ms(std::chrono::steady_clock::time_point start) {
    using namespace std::chrono;
    return static_cast<int>(duration_cast<milliseconds>(steady_clock::now() - start).count());
}

bool TimeMgr::should_stop() const {
    if (stopped_.load()) return true;
    return elapsed_ms(turn_start_) >= (budget_ms_ * 95) / 100;
}

bool TimeMgr::soft_stop() const {
    if (stopped_.load()) return true;
    return elapsed_ms(turn_start_) >= (budget_ms_ * 70) / 100;
}

} // namespace gomoku
