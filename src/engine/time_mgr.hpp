#pragma once

#include <atomic>
#include <chrono>
#include <string>

namespace gomoku {

class TimeMgr {
public:
    void set_info(const std::string& key, const std::string& value);
    void start_turn(int empty_cells);

    int budget_ms() const { return budget_ms_; }
    bool should_stop() const;
    bool soft_stop() const;
    void force_stop() { stopped_.store(true); }

private:
    int timeout_turn_ms_ = 5000;
    int timeout_match_ms_ = 0;
    int time_left_ms_ = 0;
    int budget_ms_ = 4500;
    std::chrono::steady_clock::time_point turn_start_;
    std::atomic<bool> stopped_{false};
};

} // namespace gomoku
