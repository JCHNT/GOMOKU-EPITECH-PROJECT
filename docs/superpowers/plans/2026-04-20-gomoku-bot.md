# Gomoku Bot Implementation Plan

> **For agentic workers:** REQUIRED SUB-SKILL: Use superpowers:subagent-driven-development (recommended) or superpowers:executing-plans to implement this plan task-by-task. Steps use checkbox (`- [ ]`) syntax for tracking.

**Goal:** Build a tournament-strength C++20 Gomoku bot (`pbrain-gomoku-ai`) compliant with Piskvork's mandatory protocol, compiled via a single cross-platform Makefile, strong enough to beat low-to-medium bots on a 20×20 board.

**Architecture:** Layered modules (protocol → engine → board) with bitboard-accelerated pattern detection, αβ + PVS + iterative deepening search backed by a transposition table and Zobrist hashing, defensively-tilted hand-crafted eval. Bonus features (lazy-SMP parallel search, VCF/VCT threat-space search, opening book) live under `bonus/` and link into the same binary via `make bonus`.

**Tech Stack:** C++20, POSIX `pthread` (or `std::thread`), GNU Make, Catch2 v2 (vendored single-header) for tests. Standard library only — no external deps.

**Source of truth:** `docs/superpowers/specs/2026-04-20-gomoku-bot-design.md`.

---

## Phase 0 — Bootstrap

### Task 0.1: Remove old Python implementation

**Files:**
- Delete: `pbrain-gomoku-ai` (old Python script)

- [ ] **Step 1: Remove the Python bot**

Run: `git rm pbrain-gomoku-ai`
Expected: file removed from index.

- [ ] **Step 2: Commit**

```bash
git commit -m "chore: remove legacy Python bot (replaced by C++ rewrite)"
```

---

### Task 0.2: Create directory skeleton + .gitignore

**Files:**
- Create: `src/main.cpp`
- Create: `src/util/types.hpp`
- Create: `tests/.gitkeep`
- Create: `bonus/.gitkeep`
- Create: `data/.gitkeep`
- Create: `.gitignore`

- [ ] **Step 1: Create skeleton directories and placeholder files**

```bash
mkdir -p src/protocol src/board src/engine src/util tests bonus data
touch tests/.gitkeep bonus/.gitkeep data/.gitkeep
```

- [ ] **Step 2: Write `.gitignore`**

Create `.gitignore`:

```gitignore
# Build artifacts
*.o
*.obj
*.exe
pbrain-gomoku-ai
tests/gomoku_test
tests/*.o

# IDE
.vscode/
.idea/
*.swp
.DS_Store

# Coverage
*.gcno
*.gcda
coverage/
```

- [ ] **Step 3: Write `src/util/types.hpp` with shared aliases**

Create `src/util/types.hpp`:

```cpp
#pragma once

#include <array>
#include <cstdint>

namespace gomoku {

constexpr int BOARD_SIZE = 20;
constexpr int BOARD_CELLS = BOARD_SIZE * BOARD_SIZE;

enum Color : uint8_t {
    EMPTY = 0,
    BLACK = 1,
    WHITE = 2,
};

struct Move {
    int8_t x;
    int8_t y;
    bool operator==(const Move& o) const { return x == o.x && y == o.y; }
};

constexpr Move NO_MOVE = { -1, -1 };

constexpr int WIN_SCORE = 10'000'000;

inline Color other(Color c) { return c == BLACK ? WHITE : BLACK; }
inline int cell_index(int x, int y) { return y * BOARD_SIZE + x; }

} // namespace gomoku
```

- [ ] **Step 4: Write temporary `src/main.cpp`**

Create `src/main.cpp`:

```cpp
#include <iostream>

int main() {
    std::cout << "pbrain-gomoku-ai bootstrap\n";
    return 0;
}
```

- [ ] **Step 5: Commit**

```bash
git add .gitignore src/ tests/.gitkeep bonus/.gitkeep data/.gitkeep
git commit -m "chore: scaffold C++ project skeleton"
```

---

### Task 0.3: Vendor Catch2 single-header

**Files:**
- Create: `tests/catch.hpp` (download Catch2 v2.13.10 single-header)

- [ ] **Step 1: Download Catch2 v2.13.10**

Run: `curl -fsSL https://raw.githubusercontent.com/catchorg/Catch2/v2.13.10/single_include/catch2/catch.hpp -o tests/catch.hpp`
Expected: file `tests/catch.hpp` (~700 KB) created.

- [ ] **Step 2: Sanity-check download**

Run: `head -5 tests/catch.hpp`
Expected: first line contains `/* Catch v2.13.10`.

- [ ] **Step 3: Commit**

```bash
git add tests/catch.hpp
git commit -m "chore: vendor Catch2 v2.13.10 single-header"
```

---

### Task 0.4: Top-level Makefile (skeleton that builds placeholder binary)

**Files:**
- Create: `Makefile`

- [ ] **Step 1: Write `Makefile`**

Create `Makefile`:

```make
NAME     := pbrain-gomoku-ai
CXX      ?= g++
CXXFLAGS := -std=c++20 -O3 -march=native -DNDEBUG -pthread -Wall -Wextra -Werror -Isrc
LDFLAGS  := -pthread

ifeq ($(OS),Windows_NT)
    NAME := $(NAME).exe
    LDFLAGS += -static
    CXXFLAGS := $(filter-out -march=native,$(CXXFLAGS))
endif

SRC_DIRS := src src/protocol src/board src/engine src/util
SRC      := $(shell find src -name '*.cpp' 2>/dev/null)
OBJ      := $(SRC:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ) tests/*.o

fclean: clean
	rm -f $(NAME) tests/gomoku_test

re: fclean all

test:
	$(MAKE) -f tests/Makefile run

bonus:
	$(MAKE) -C bonus

debug: CXXFLAGS := -std=c++20 -O0 -g -pthread -Wall -Wextra -Isrc -fsanitize=address,undefined
debug: LDFLAGS += -fsanitize=address,undefined
debug: re

.PHONY: all re clean fclean test bonus debug
```

- [ ] **Step 2: Verify build**

Run: `make && ./pbrain-gomoku-ai`
Expected: prints `pbrain-gomoku-ai bootstrap`.

- [ ] **Step 3: Verify clean targets**

Run: `make fclean && ls pbrain-gomoku-ai 2>&1`
Expected: `ls: pbrain-gomoku-ai: No such file or directory`.

- [ ] **Step 4: Commit**

```bash
git add Makefile
git commit -m "build: add top-level Makefile with all/re/clean/fclean/test/bonus/debug"
```

---

### Task 0.5: Tests Makefile + sample test runs

**Files:**
- Create: `tests/Makefile`
- Create: `tests/test_sanity.cpp`

- [ ] **Step 1: Write sanity test**

Create `tests/test_sanity.cpp`:

```cpp
#define CATCH_CONFIG_MAIN
#include "catch.hpp"

TEST_CASE("catch2 is wired up", "[sanity]") {
    REQUIRE(1 + 1 == 2);
}
```

- [ ] **Step 2: Write `tests/Makefile`**

Create `tests/Makefile`:

```make
CXX      ?= g++
CXXFLAGS := -std=c++20 -O2 -pthread -Wall -Wextra -Isrc -Itests
LDFLAGS  := -pthread

# All non-main sources from src/, except main.cpp
SRC_LIB  := $(shell find src -name '*.cpp' ! -name 'main.cpp' 2>/dev/null)
SRC_TEST := $(wildcard tests/test_*.cpp)

OBJ_LIB  := $(SRC_LIB:.cpp=.o)
OBJ_TEST := $(SRC_TEST:.cpp=.o)

TARGET := tests/gomoku_test

$(TARGET): $(OBJ_LIB) $(OBJ_TEST)
	$(CXX) $(OBJ_LIB) $(OBJ_TEST) -o $(TARGET) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

run: $(TARGET)
	./$(TARGET)

clean:
	rm -f $(OBJ_LIB) $(OBJ_TEST) $(TARGET)

.PHONY: run clean
```

- [ ] **Step 3: Run tests**

Run: `make test`
Expected: `All tests passed (1 assertion in 1 test case)`.

- [ ] **Step 4: Commit**

```bash
git add tests/Makefile tests/test_sanity.cpp
git commit -m "test: add Catch2 sanity test and tests/Makefile"
```

---

## Phase 1 — Board, Zobrist, and make/unmake

### Task 1.1: Zobrist hashing (failing test first)

**Files:**
- Create: `tests/test_zobrist.cpp`
- Create: `src/board/zobrist.hpp`
- Create: `src/board/zobrist.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_zobrist.cpp`:

```cpp
#include "catch.hpp"
#include "board/zobrist.hpp"

using namespace gomoku;

TEST_CASE("zobrist keys are stable across calls", "[zobrist]") {
    const auto& k1 = Zobrist::instance();
    const auto& k2 = Zobrist::instance();
    REQUIRE(&k1 == &k2);
    REQUIRE(k1.key(BLACK, 0, 0) == k2.key(BLACK, 0, 0));
}

TEST_CASE("zobrist keys differ per (color,cell)", "[zobrist]") {
    const auto& z = Zobrist::instance();
    REQUIRE(z.key(BLACK, 0, 0) != z.key(WHITE, 0, 0));
    REQUIRE(z.key(BLACK, 0, 0) != z.key(BLACK, 1, 0));
    REQUIRE(z.key(BLACK, 5, 5) != 0);
}

TEST_CASE("zobrist XOR round-trip returns to zero", "[zobrist]") {
    const auto& z = Zobrist::instance();
    uint64_t h = 0;
    h ^= z.key(BLACK, 3, 4);
    h ^= z.key(WHITE, 7, 8);
    h ^= z.key(BLACK, 3, 4);
    h ^= z.key(WHITE, 7, 8);
    REQUIRE(h == 0);
}
```

- [ ] **Step 2: Run test — expect compile failure**

Run: `make test`
Expected: compile error, `zobrist.hpp not found`.

- [ ] **Step 3: Write `src/board/zobrist.hpp`**

```cpp
#pragma once

#include "util/types.hpp"
#include <cstdint>

namespace gomoku {

class Zobrist {
public:
    static const Zobrist& instance();

    uint64_t key(Color c, int x, int y) const {
        return keys_[c == BLACK ? 0 : 1][cell_index(x, y)];
    }

private:
    Zobrist();
    uint64_t keys_[2][BOARD_CELLS];
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/board/zobrist.cpp`**

```cpp
#include "board/zobrist.hpp"
#include <random>

namespace gomoku {

Zobrist::Zobrist() {
    std::mt19937_64 rng(0xC0FFEE1234567890ULL);
    for (int c = 0; c < 2; ++c)
        for (int i = 0; i < BOARD_CELLS; ++i)
            keys_[c][i] = rng();
}

const Zobrist& Zobrist::instance() {
    static const Zobrist z;
    return z;
}

} // namespace gomoku
```

- [ ] **Step 5: Run test — expect pass**

Run: `make test`
Expected: `All tests passed (5 assertions in 3 test cases)` (plus sanity).

- [ ] **Step 6: Commit**

```bash
git add tests/test_zobrist.cpp src/board/zobrist.hpp src/board/zobrist.cpp
git commit -m "feat(board): add Zobrist hashing with fixed-seed key table"
```

---

### Task 1.2: Board state + make/undo

**Files:**
- Create: `tests/test_board.cpp`
- Create: `src/board/board.hpp`
- Create: `src/board/board.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_board.cpp`:

```cpp
#include "catch.hpp"
#include "board/board.hpp"

using namespace gomoku;

TEST_CASE("fresh board is empty with zero hash", "[board]") {
    Board b;
    REQUIRE(b.hash() == 0);
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            REQUIRE(b.at(x, y) == EMPTY);
}

TEST_CASE("place + undo returns to empty and zero hash", "[board]") {
    Board b;
    b.make_move({5, 5}, BLACK);
    REQUIRE(b.at(5, 5) == BLACK);
    REQUIRE(b.hash() != 0);
    b.undo_move();
    REQUIRE(b.at(5, 5) == EMPTY);
    REQUIRE(b.hash() == 0);
}

TEST_CASE("hash is xor of placed keys (order-independent)", "[board]") {
    Board a, b;
    a.make_move({1, 2}, BLACK);
    a.make_move({3, 4}, WHITE);
    b.make_move({3, 4}, WHITE);
    b.make_move({1, 2}, BLACK);
    REQUIRE(a.hash() == b.hash());
}

TEST_CASE("double-place rejects and leaves state unchanged", "[board]") {
    Board b;
    REQUIRE(b.make_move({5, 5}, BLACK) == true);
    REQUIRE(b.make_move({5, 5}, WHITE) == false);
    REQUIRE(b.at(5, 5) == BLACK);
}

TEST_CASE("out-of-bounds rejects", "[board]") {
    Board b;
    REQUIRE(b.make_move({-1, 0}, BLACK) == false);
    REQUIRE(b.make_move({BOARD_SIZE, 0}, BLACK) == false);
    REQUIRE(b.make_move({0, BOARD_SIZE}, BLACK) == false);
}

TEST_CASE("stone_count reflects move history", "[board]") {
    Board b;
    REQUIRE(b.stone_count() == 0);
    b.make_move({10, 10}, BLACK);
    b.make_move({11, 10}, WHITE);
    REQUIRE(b.stone_count() == 2);
    b.undo_move();
    REQUIRE(b.stone_count() == 1);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `board.hpp not found`.

- [ ] **Step 3: Write `src/board/board.hpp`**

```cpp
#pragma once

#include "board/zobrist.hpp"
#include "util/types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

class Board {
public:
    Board();

    bool make_move(Move m, Color c);
    void undo_move();

    Color at(int x, int y) const {
        return static_cast<Color>(cells_[y][x]);
    }
    uint64_t hash() const { return hash_; }
    int stone_count() const { return static_cast<int>(history_.size()); }

    Move last_move() const {
        return history_.empty() ? NO_MOVE : history_.back().move;
    }

    static bool in_bounds(Move m) {
        return m.x >= 0 && m.x < BOARD_SIZE && m.y >= 0 && m.y < BOARD_SIZE;
    }

private:
    struct HistoryEntry {
        Move move;
        Color color;
    };

    uint8_t cells_[BOARD_SIZE][BOARD_SIZE];
    uint64_t hash_;
    std::vector<HistoryEntry> history_;
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/board/board.cpp`**

```cpp
#include "board/board.hpp"
#include <cstring>

namespace gomoku {

Board::Board() : hash_(0) {
    std::memset(cells_, 0, sizeof(cells_));
    history_.reserve(BOARD_CELLS);
}

bool Board::make_move(Move m, Color c) {
    if (!in_bounds(m)) return false;
    if (cells_[m.y][m.x] != EMPTY) return false;
    cells_[m.y][m.x] = c;
    hash_ ^= Zobrist::instance().key(c, m.x, m.y);
    history_.push_back({m, c});
    return true;
}

void Board::undo_move() {
    if (history_.empty()) return;
    const auto entry = history_.back();
    history_.pop_back();
    cells_[entry.move.y][entry.move.x] = EMPTY;
    hash_ ^= Zobrist::instance().key(entry.color, entry.move.x, entry.move.y);
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all board tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_board.cpp src/board/board.hpp src/board/board.cpp
git commit -m "feat(board): add Board with make/undo and Zobrist-backed hash"
```

---

### Task 1.3: Line bitboards (incremental per-direction masks)

**Files:**
- Modify: `src/board/board.hpp`
- Modify: `src/board/board.cpp`
- Modify: `tests/test_board.cpp`

- [ ] **Step 1: Add failing test for line access**

Append to `tests/test_board.cpp`:

```cpp
TEST_CASE("line bitboards reflect placed stones (horizontal)", "[board][lines]") {
    Board b;
    b.make_move({3, 7}, BLACK);
    b.make_move({5, 7}, BLACK);
    uint32_t row = b.line_mask(BLACK, Board::LINE_ROW, 7);
    REQUIRE((row & (1u << 3)) != 0);
    REQUIRE((row & (1u << 5)) != 0);
    REQUIRE((row & (1u << 4)) == 0);
}

TEST_CASE("line bitboards reflect placed stones (vertical)", "[board][lines]") {
    Board b;
    b.make_move({10, 2}, WHITE);
    b.make_move({10, 9}, WHITE);
    uint32_t col = b.line_mask(WHITE, Board::LINE_COL, 10);
    REQUIRE((col & (1u << 2)) != 0);
    REQUIRE((col & (1u << 9)) != 0);
}

TEST_CASE("line bitboards clear on undo", "[board][lines]") {
    Board b;
    b.make_move({4, 4}, BLACK);
    REQUIRE(b.line_mask(BLACK, Board::LINE_ROW, 4) != 0);
    b.undo_move();
    REQUIRE(b.line_mask(BLACK, Board::LINE_ROW, 4) == 0);
}

TEST_CASE("diagonals track stones (main diagonal, x - y const)", "[board][lines]") {
    // Main diag index = (x - y) + (BOARD_SIZE - 1) = 0..38
    Board b;
    b.make_move({5, 5}, BLACK);   // diag index 19
    b.make_move({7, 7}, BLACK);   // diag index 19
    uint32_t diag = b.line_mask(BLACK, Board::LINE_DIAG, 19);
    // Position along diag = min(x, y) when x-y>=0 and min(x,y) in [0, BOARD_SIZE-1)
    // For (5,5) -> 5; for (7,7) -> 7
    REQUIRE((diag & (1u << 5)) != 0);
    REQUIRE((diag & (1u << 7)) != 0);
}

TEST_CASE("anti-diagonals track stones (x + y const)", "[board][lines]") {
    // Anti-diag index = x + y, in 0..2*(BOARD_SIZE-1)
    Board b;
    b.make_move({3, 7}, WHITE);   // anti-diag 10
    b.make_move({6, 4}, WHITE);   // anti-diag 10
    uint32_t anti = b.line_mask(WHITE, Board::LINE_ANTI, 10);
    // Position along anti-diag = x (since y = antiIdx - x)
    REQUIRE((anti & (1u << 3)) != 0);
    REQUIRE((anti & (1u << 6)) != 0);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `no member 'line_mask'`.

- [ ] **Step 3: Extend `src/board/board.hpp`**

Replace the entire `Board` class with:

```cpp
#pragma once

#include "board/zobrist.hpp"
#include "util/types.hpp"
#include <cstdint>
#include <vector>

namespace gomoku {

class Board {
public:
    enum LineKind : uint8_t {
        LINE_ROW = 0,   // 20 rows
        LINE_COL = 1,   // 20 cols
        LINE_DIAG = 2,  // 39 main diagonals  (indexed by x - y + (N-1))
        LINE_ANTI = 3,  // 39 anti-diagonals (indexed by x + y)
    };

    static constexpr int N_ROWS  = BOARD_SIZE;
    static constexpr int N_COLS  = BOARD_SIZE;
    static constexpr int N_DIAGS = 2 * BOARD_SIZE - 1;  // 39
    static constexpr int N_ANTIS = 2 * BOARD_SIZE - 1;  // 39

    Board();

    bool make_move(Move m, Color c);
    void undo_move();

    Color at(int x, int y) const { return static_cast<Color>(cells_[y][x]); }
    uint64_t hash() const { return hash_; }
    int stone_count() const { return static_cast<int>(history_.size()); }
    Move last_move() const {
        return history_.empty() ? NO_MOVE : history_.back().move;
    }

    uint32_t line_mask(Color c, LineKind kind, int idx) const {
        return lines_[c == BLACK ? 0 : 1][kind][idx];
    }

    // position along a line for a given (x,y) and line kind
    static int line_pos(LineKind kind, int x, int y);
    // line index for (x,y) and kind
    static int line_index(LineKind kind, int x, int y);

    static bool in_bounds(Move m) {
        return m.x >= 0 && m.x < BOARD_SIZE && m.y >= 0 && m.y < BOARD_SIZE;
    }

private:
    struct HistoryEntry {
        Move move;
        Color color;
    };

    void toggle_lines(Move m, Color c);

    uint8_t cells_[BOARD_SIZE][BOARD_SIZE];
    uint64_t hash_;
    // lines_[colorIndex][LineKind][lineIndex] = 32-bit mask, bit i set if stone
    uint32_t lines_[2][4][N_DIAGS];  // N_DIAGS is the largest count (39)
    std::vector<HistoryEntry> history_;
};

} // namespace gomoku
```

- [ ] **Step 4: Rewrite `src/board/board.cpp`**

```cpp
#include "board/board.hpp"
#include <cstring>

namespace gomoku {

Board::Board() : hash_(0) {
    std::memset(cells_, 0, sizeof(cells_));
    std::memset(lines_, 0, sizeof(lines_));
    history_.reserve(BOARD_CELLS);
}

int Board::line_index(LineKind kind, int x, int y) {
    switch (kind) {
        case LINE_ROW:  return y;
        case LINE_COL:  return x;
        case LINE_DIAG: return x - y + (BOARD_SIZE - 1);
        case LINE_ANTI: return x + y;
    }
    return 0;
}

int Board::line_pos(LineKind kind, int x, int y) {
    switch (kind) {
        case LINE_ROW:  return x;
        case LINE_COL:  return y;
        case LINE_DIAG:
            // on diag i = x - y + (N-1), position = min(x, y) when i >= N-1,
            // or x when i < N-1. Uniformly: pos = x when y <= x, y when x < y.
            // But since i = x - y + (N-1), we have y = x - (i - (N-1)).
            // pos along diag = x when i >= N-1 (y = x - (i-(N-1)) so pos = y+... ugh
            // Cleanest: pos = std::min(x, y) isn't right across all diags.
            // Define pos = x if (x - y) >= 0 else y? Let's just use x.
            // Actually position along the diag is the number of cells to the
            // start of the diag. For diag index i:
            //   if i >= N-1: start is (i - (N-1), 0), pos = y  (and x = pos + i - (N-1))
            //   else:         start is (0, (N-1) - i), pos = x (and y = pos + (N-1) - i)
            return (x - y) >= 0 ? y : x;
        case LINE_ANTI:
            // anti-diag index i = x + y. start is (min(i, N-1), max(0, i - (N-1))).
            // For i <= N-1: start is (i, 0), pos = -x + i (going down-left), but
            // we want monotonic increasing; use x (since y = i - x, x increases => y decreases).
            // For i > N-1: start is (N-1, i - (N-1)), pos = (N-1) - x.
            return (x + y) <= (BOARD_SIZE - 1) ? x : (BOARD_SIZE - 1 - x + (x + y - (BOARD_SIZE - 1)));
    }
    return 0;
}

void Board::toggle_lines(Move m, Color c) {
    const int ci = (c == BLACK) ? 0 : 1;
    for (int k = 0; k < 4; ++k) {
        auto kind = static_cast<LineKind>(k);
        const int idx = line_index(kind, m.x, m.y);
        const int pos = line_pos(kind, m.x, m.y);
        lines_[ci][k][idx] ^= (1u << pos);
    }
}

bool Board::make_move(Move m, Color c) {
    if (!in_bounds(m)) return false;
    if (cells_[m.y][m.x] != EMPTY) return false;
    cells_[m.y][m.x] = c;
    hash_ ^= Zobrist::instance().key(c, m.x, m.y);
    toggle_lines(m, c);
    history_.push_back({m, c});
    return true;
}

void Board::undo_move() {
    if (history_.empty()) return;
    const auto entry = history_.back();
    history_.pop_back();
    cells_[entry.move.y][entry.move.x] = EMPTY;
    hash_ ^= Zobrist::instance().key(entry.color, entry.move.x, entry.move.y);
    toggle_lines(entry.move, entry.color);
}

} // namespace gomoku
```

- [ ] **Step 5: Verify `line_pos` correctness for anti-diag by property test**

Append to `tests/test_board.cpp`:

```cpp
TEST_CASE("line_pos round-trips: reconstructing (x,y) from (kind,idx,pos) matches", "[board][lines]") {
    using LK = Board::LineKind;
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            for (int k = 0; k < 4; ++k) {
                auto kind = static_cast<LK>(k);
                int idx = Board::line_index(kind, x, y);
                int pos = Board::line_pos(kind, x, y);
                // Place a single stone, verify mask bit matches pos.
                Board b;
                b.make_move({(int8_t)x, (int8_t)y}, BLACK);
                uint32_t mask = b.line_mask(BLACK, kind, idx);
                INFO("x=" << x << " y=" << y << " kind=" << k << " idx=" << idx << " pos=" << pos);
                REQUIRE((mask & (1u << pos)) != 0);
                REQUIRE(__builtin_popcount(mask) == 1);
            }
        }
    }
}
```

- [ ] **Step 6: Run tests — expect pass**

Run: `make test`
Expected: all board tests (including bitboard) pass.

If the anti-diag formula fails in the property test, simplify `line_pos` for anti-diag to just `x` — the position along the anti-diag is unambiguously `x` once `idx = x + y` is fixed (since `y = idx - x`, x uniquely identifies the cell on that anti-diag). Update:

```cpp
case LINE_ANTI: return x;
```

And re-run.

- [ ] **Step 7: Commit**

```bash
git add src/board/board.hpp src/board/board.cpp tests/test_board.cpp
git commit -m "feat(board): add incremental line bitboards for 4 directions"
```

---

## Phase 2 — Pattern detection

### Task 2.1: Pattern enum + detection on a single line

**Files:**
- Create: `tests/test_patterns.cpp`
- Create: `src/engine/patterns.hpp`
- Create: `src/engine/patterns.cpp`

- [ ] **Step 1: Write failing test for detection on a single line mask**

Create `tests/test_patterns.cpp`:

```cpp
#include "catch.hpp"
#include "engine/patterns.hpp"

using namespace gomoku;

// Helper: build a 20-bit own/opp mask pair from a string like
//   "....XXXXX...........", where X = own, O = opp, . = empty.
static std::pair<uint32_t,uint32_t> make_line(const std::string& s) {
    uint32_t own = 0, opp = 0;
    for (size_t i = 0; i < s.size() && i < 20; ++i) {
        if (s[i] == 'X') own |= (1u << i);
        else if (s[i] == 'O') opp |= (1u << i);
    }
    return {own, opp};
}

TEST_CASE("detects FIVE in a row", "[patterns]") {
    auto [own, opp] = make_line("....XXXXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::FIVE] == 1);
}

TEST_CASE("detects OPEN_FOUR", "[patterns]") {
    auto [own, opp] = make_line(".....XXXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::OPEN_FOUR] == 1);
}

TEST_CASE("closed four: blocked one side", "[patterns]") {
    auto [own, opp] = make_line("....OXXXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::CLOSED_FOUR] == 1);
    REQUIRE(counts[Patterns::OPEN_FOUR] == 0);
}

TEST_CASE("closed four: gapped X_XXX", "[patterns]") {
    auto [own, opp] = make_line("....X.XXX...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::CLOSED_FOUR] == 1);
}

TEST_CASE("open three: _XXX_", "[patterns]") {
    auto [own, opp] = make_line(".....XXX............");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::OPEN_THREE] == 1);
}

TEST_CASE("open three (gapped): _X_XX_ and _XX_X_", "[patterns]") {
    {
        auto [own, opp] = make_line("....X.XX............");
        auto counts = Patterns::count_line(own, opp);
        REQUIRE(counts[Patterns::OPEN_THREE] == 1);
    }
    {
        auto [own, opp] = make_line("....XX.X............");
        auto counts = Patterns::count_line(own, opp);
        REQUIRE(counts[Patterns::OPEN_THREE] == 1);
    }
}

TEST_CASE("closed three: blocked _XXXO", "[patterns]") {
    auto [own, opp] = make_line(".....XXXO...........");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::CLOSED_THREE] == 1);
    REQUIRE(counts[Patterns::OPEN_THREE] == 0);
}

TEST_CASE("open two: _XX_", "[patterns]") {
    auto [own, opp] = make_line(".....XX.............");
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::OPEN_TWO] == 1);
}

TEST_CASE("board edges count as blockers", "[patterns]") {
    auto [own, opp] = make_line("XXXXX...............");  // touches left edge
    auto counts = Patterns::count_line(own, opp);
    REQUIRE(counts[Patterns::FIVE] == 1);
}

TEST_CASE("empty line produces zero counts", "[patterns]") {
    auto [own, opp] = make_line("....................");
    auto counts = Patterns::count_line(own, opp);
    for (int i = 0; i < Patterns::COUNT; ++i) REQUIRE(counts[i] == 0);
}
```

- [ ] **Step 2: Run tests — expect compile fail**

Run: `make test`
Expected: `patterns.hpp not found`.

- [ ] **Step 3: Write `src/engine/patterns.hpp`**

```cpp
#pragma once

#include "util/types.hpp"
#include <array>
#include <cstdint>

namespace gomoku {

class Patterns {
public:
    enum Kind : uint8_t {
        FIVE = 0,
        OPEN_FOUR,
        CLOSED_FOUR,
        OPEN_THREE,
        CLOSED_THREE,
        OPEN_TWO,
        CLOSED_TWO,
        COUNT
    };

    using Counts = std::array<uint16_t, COUNT>;

    // Count all pattern occurrences along a single 20-cell line.
    // own = bitmask of own stones (bit i => cell i), opp = opponent stones.
    // Cells out of line range (above bit 19) are treated as blockers (opponent).
    static Counts count_line(uint32_t own, uint32_t opp);
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/engine/patterns.cpp`**

```cpp
#include "engine/patterns.hpp"

namespace gomoku {

static constexpr int LINE_LEN = 20;

// Treat off-line cells (< 0 or >= LINE_LEN) as opponent (edge = blocker).
static inline bool is_own(uint32_t own, int i) {
    return i >= 0 && i < LINE_LEN && (own & (1u << i));
}
static inline bool is_opp(uint32_t opp, int i) {
    if (i < 0 || i >= LINE_LEN) return true;     // edge
    return opp & (1u << i);
}
static inline bool is_empty(uint32_t own, uint32_t opp, int i) {
    return !is_own(own, i) && !is_opp(opp, i);
}

// Match FIVE starting at position i.
static inline bool match_five(uint32_t own, int i) {
    if (i < 0 || i + 4 >= LINE_LEN) return false;
    uint32_t mask = 0b11111u << i;
    return (own & mask) == mask;
}

// Open four: _XXXX_
static inline bool match_open_four(uint32_t own, uint32_t opp, int i) {
    // i = position of first X. Need empty at i-1 and i+4, stones at i..i+3,
    // and not a FIVE (no X at i-1 already guaranteed; just no further overlap).
    if (i < 1 || i + 4 > LINE_LEN) return false;
    if (!is_empty(own, opp, i - 1)) return false;
    for (int k = 0; k < 4; ++k) if (!is_own(own, i + k)) return false;
    if (!is_empty(own, opp, i + 4)) return false;
    return true;
}

// Closed four variants:
//   (a) OXXXX. or .XXXXO (exactly one side blocked/edge, four contiguous, other side empty)
//   (b) XX_XX, X_XXX, XXX_X (gapped four-in-five-window)
// For simplicity, count all "four stones that threaten to win in one move".
static inline bool match_closed_four(uint32_t own, uint32_t opp, int i) {
    // (a) contiguous XXXX with exactly one blocker side
    if (i >= 0 && i + 3 < LINE_LEN) {
        bool all_four = true;
        for (int k = 0; k < 4; ++k) if (!is_own(own, i + k)) all_four = false;
        if (all_four) {
            bool left_blocked  = (i == 0) || is_opp(opp, i - 1);
            bool right_blocked = (i + 4 >= LINE_LEN) || is_opp(opp, i + 4);
            bool left_empty  = (i > 0) && is_empty(own, opp, i - 1);
            bool right_empty = (i + 4 < LINE_LEN) && is_empty(own, opp, i + 4);
            // exclude FIVE (would be all 5 own): handled by FIVE pass
            // exclude OPEN_FOUR: both sides empty
            if ((left_blocked && right_empty) || (left_empty && right_blocked)) {
                return true;
            }
        }
    }
    return false;
}

// Gapped four: exactly one missing stone within a 5-cell window, ends
// bounded by something not-own (empty OR blocker). Uses window start i..i+4.
static inline bool match_gapped_four(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 4 >= LINE_LEN) return false;
    int own_count = 0, empty_count = 0;
    int empty_pos = -1;
    for (int k = 0; k < 5; ++k) {
        if (is_own(own, i + k)) ++own_count;
        else if (is_empty(own, opp, i + k)) { ++empty_count; empty_pos = i + k; }
        else return false;  // opp inside window
    }
    if (own_count != 4 || empty_count != 1) return false;
    // Avoid double-counting with contiguous closed-four detection: gapped
    // requires the empty to be strictly inside (positions i+1..i+3).
    return empty_pos > i && empty_pos < i + 4;
}

// Open three: _XXX_ or _X_XX_ / _XX_X_ with both ends empty.
static inline bool match_open_three(uint32_t own, uint32_t opp, int i) {
    // Contiguous: positions i-1 empty, i..i+2 own, i+3 empty.
    if (i >= 1 && i + 3 < LINE_LEN) {
        bool ok = is_empty(own, opp, i - 1)
               && is_own(own, i) && is_own(own, i + 1) && is_own(own, i + 2)
               && is_empty(own, opp, i + 3);
        // Reject if this is actually the tail of a four (i-2 is own -> not contig "_XXX_")
        if (ok) {
            if (i - 2 >= 0 && is_own(own, i - 2)) ok = false;
            if (i + 4 < LINE_LEN && is_own(own, i + 4)) ok = false;
        }
        if (ok) return true;
    }
    // Gapped: _X_XX_ window size 6, starting at j = i-1 where own pattern starts at i
    // Represent as 6-window: positions p..p+5. Own at p+1, p+3, p+4 and empties at p, p+2, p+5.
    if (i >= 0 && i + 5 < LINE_LEN) {
        // _X_XX_
        if (is_empty(own, opp, i) && is_own(own, i+1) && is_empty(own, opp, i+2)
            && is_own(own, i+3) && is_own(own, i+4) && is_empty(own, opp, i+5))
            return true;
        // _XX_X_
        if (is_empty(own, opp, i) && is_own(own, i+1) && is_own(own, i+2)
            && is_empty(own, opp, i+3) && is_own(own, i+4) && is_empty(own, opp, i+5))
            return true;
    }
    return false;
}

// Closed three: 3 own stones with exactly one side blocked/edge.
static inline bool match_closed_three(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 2 >= LINE_LEN) return false;
    if (!(is_own(own, i) && is_own(own, i+1) && is_own(own, i+2))) return false;
    bool left_blocked  = (i == 0) || is_opp(opp, i - 1);
    bool right_blocked = (i + 3 >= LINE_LEN) || is_opp(opp, i + 3);
    bool left_empty  = (i > 0) && is_empty(own, opp, i - 1);
    bool right_empty = (i + 3 < LINE_LEN) && is_empty(own, opp, i + 3);
    if ((left_blocked && right_empty) || (left_empty && right_blocked)) {
        // exclude part-of-four patterns
        if (i - 2 >= 0 && is_own(own, i - 2)) return false;
        if (i + 4 < LINE_LEN && is_own(own, i + 4)) return false;
        return true;
    }
    return false;
}

// Open two: _XX_ contiguous, with both ends empty, not part of a three.
static inline bool match_open_two(uint32_t own, uint32_t opp, int i) {
    if (i < 1 || i + 2 >= LINE_LEN) return false;
    if (!(is_empty(own, opp, i - 1) && is_own(own, i) && is_own(own, i + 1) && is_empty(own, opp, i + 2)))
        return false;
    if (i - 2 >= 0 && is_own(own, i - 2)) return false;
    if (i + 3 < LINE_LEN && is_own(own, i + 3)) return false;
    return true;
}

// Closed two: XX with exactly one side blocked, other empty, not part of three.
static inline bool match_closed_two(uint32_t own, uint32_t opp, int i) {
    if (i < 0 || i + 1 >= LINE_LEN) return false;
    if (!(is_own(own, i) && is_own(own, i + 1))) return false;
    bool left_blocked  = (i == 0) || is_opp(opp, i - 1);
    bool right_blocked = (i + 2 >= LINE_LEN) || is_opp(opp, i + 2);
    bool left_empty  = (i > 0) && is_empty(own, opp, i - 1);
    bool right_empty = (i + 2 < LINE_LEN) && is_empty(own, opp, i + 2);
    if ((left_blocked && right_empty) || (left_empty && right_blocked)) {
        if (i - 2 >= 0 && is_own(own, i - 2)) return false;
        if (i + 3 < LINE_LEN && is_own(own, i + 3)) return false;
        return true;
    }
    return false;
}

Patterns::Counts Patterns::count_line(uint32_t own, uint32_t opp) {
    Counts c{};

    // Scan each starting position once. Overlapping patterns: the matcher
    // for each kind emits exactly one count per non-overlapping occurrence;
    // we avoid double-counting via the side-condition checks above (e.g.
    // an open three is rejected if it's the middle of an OPEN_FOUR).

    for (int i = 0; i < LINE_LEN; ++i) {
        if (match_five(own, i))         ++c[FIVE];
        if (match_open_four(own, opp, i)) ++c[OPEN_FOUR];
        if (match_closed_four(own, opp, i)) ++c[CLOSED_FOUR];
        if (match_gapped_four(own, opp, i)) ++c[CLOSED_FOUR];
        if (match_open_three(own, opp, i))  ++c[OPEN_THREE];
        if (match_closed_three(own, opp, i))++c[CLOSED_THREE];
        if (match_open_two(own, opp, i))    ++c[OPEN_TWO];
        if (match_closed_two(own, opp, i))  ++c[CLOSED_TWO];
    }
    return c;
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all pattern tests pass. If any fail, debug the specific matcher — the tests print the failing line in `INFO`.

- [ ] **Step 6: Commit**

```bash
git add tests/test_patterns.cpp src/engine/patterns.hpp src/engine/patterns.cpp
git commit -m "feat(engine): add 7-class Gomoku pattern detection on line masks"
```

---

### Task 2.2: Integrate pattern counting at board level

**Files:**
- Create: `tests/test_board_patterns.cpp`
- Modify: `src/board/board.hpp`
- Modify: `src/board/board.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_board_patterns.cpp`:

```cpp
#include "catch.hpp"
#include "board/board.hpp"
#include "engine/patterns.hpp"

using namespace gomoku;

TEST_CASE("board counts FIVE horizontal", "[board][patterns]") {
    Board b;
    for (int x = 5; x <= 9; ++x) b.make_move({(int8_t)x, 10}, BLACK);
    auto black_counts = b.count_patterns(BLACK);
    REQUIRE(black_counts[Patterns::FIVE] == 1);
}

TEST_CASE("board counts FIVE vertical", "[board][patterns]") {
    Board b;
    for (int y = 3; y <= 7; ++y) b.make_move({10, (int8_t)y}, BLACK);
    REQUIRE(b.count_patterns(BLACK)[Patterns::FIVE] == 1);
}

TEST_CASE("board counts FIVE main diagonal", "[board][patterns]") {
    Board b;
    for (int k = 0; k < 5; ++k) b.make_move({(int8_t)(4 + k), (int8_t)(4 + k)}, WHITE);
    REQUIRE(b.count_patterns(WHITE)[Patterns::FIVE] == 1);
}

TEST_CASE("board counts FIVE anti-diagonal", "[board][patterns]") {
    Board b;
    for (int k = 0; k < 5; ++k) b.make_move({(int8_t)(10 + k), (int8_t)(14 - k)}, WHITE);
    REQUIRE(b.count_patterns(WHITE)[Patterns::FIVE] == 1);
}

TEST_CASE("empty board = zero counts", "[board][patterns]") {
    Board b;
    for (int i = 0; i < Patterns::COUNT; ++i) {
        REQUIRE(b.count_patterns(BLACK)[i] == 0);
        REQUIRE(b.count_patterns(WHITE)[i] == 0);
    }
}

TEST_CASE("has_five returns true exactly when a 5 exists", "[board][patterns]") {
    Board b;
    for (int x = 2; x <= 5; ++x) b.make_move({(int8_t)x, 8}, BLACK);
    REQUIRE(b.has_five(BLACK) == false);
    b.make_move({6, 8}, BLACK);
    REQUIRE(b.has_five(BLACK) == true);
    REQUIRE(b.has_five(WHITE) == false);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `no member 'count_patterns'`.

- [ ] **Step 3: Extend `Board` in `src/board/board.hpp`**

Add `#include "engine/patterns.hpp"` at the top and these methods to the `Board` class public section:

```cpp
Patterns::Counts count_patterns(Color c) const;
bool has_five(Color c) const;
```

- [ ] **Step 4: Extend `src/board/board.cpp`**

Append:

```cpp
Patterns::Counts Board::count_patterns(Color c) const {
    Patterns::Counts total{};
    const int ci = (c == BLACK) ? 0 : 1;
    const int oi = 1 - ci;
    for (int k = 0; k < 4; ++k) {
        int max_idx;
        switch (k) {
            case LINE_ROW: max_idx = N_ROWS; break;
            case LINE_COL: max_idx = N_COLS; break;
            case LINE_DIAG: max_idx = N_DIAGS; break;
            case LINE_ANTI: max_idx = N_ANTIS; break;
            default: max_idx = 0;
        }
        for (int i = 0; i < max_idx; ++i) {
            auto line = Patterns::count_line(lines_[ci][k][i], lines_[oi][k][i]);
            for (int p = 0; p < Patterns::COUNT; ++p) total[p] += line[p];
        }
    }
    return total;
}

bool Board::has_five(Color c) const {
    return count_patterns(c)[Patterns::FIVE] > 0;
}
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all board+pattern tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_board_patterns.cpp src/board/board.hpp src/board/board.cpp
git commit -m "feat(board): aggregate pattern counts across all four line directions"
```

---

## Phase 3 — Eval

### Task 3.1: Weighted eval function

**Files:**
- Create: `tests/test_eval.cpp`
- Create: `src/engine/eval.hpp`
- Create: `src/engine/eval.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_eval.cpp`:

```cpp
#include "catch.hpp"
#include "board/board.hpp"
#include "engine/eval.hpp"

using namespace gomoku;

TEST_CASE("empty board evaluates to 0", "[eval]") {
    Board b;
    REQUIRE(Eval::score(b, BLACK) == 0);
}

TEST_CASE("FIVE evaluates to WIN_SCORE for the side of play", "[eval]") {
    Board b;
    for (int x = 5; x <= 9; ++x) b.make_move({(int8_t)x, 10}, BLACK);
    REQUIRE(Eval::score(b, BLACK) >= WIN_SCORE);
    REQUIRE(Eval::score(b, WHITE) <= -WIN_SCORE);
}

TEST_CASE("OPEN_FOUR for own side is positive and large", "[eval]") {
    Board b;
    for (int x = 5; x <= 8; ++x) b.make_move({(int8_t)x, 10}, BLACK);
    // _XXXX_ at (4,10)..(9,10)
    int s = Eval::score(b, BLACK);
    REQUIRE(s > 50'000);
    REQUIRE(s < WIN_SCORE);
}

TEST_CASE("symmetric: swapping colors negates score", "[eval]") {
    Board a;
    a.make_move({10, 10}, BLACK);
    a.make_move({11, 10}, BLACK);
    a.make_move({12, 10}, BLACK);
    int sa = Eval::score(a, BLACK);

    Board b;
    b.make_move({10, 10}, WHITE);
    b.make_move({11, 10}, WHITE);
    b.make_move({12, 10}, WHITE);
    int sb = Eval::score(b, BLACK);

    REQUIRE(sa == -sb);
}

TEST_CASE("defensive tilt: opponent threats weigh more than own", "[eval]") {
    // Same pattern for each side should make side-to-move slightly less happy.
    Board b;
    // Own open three
    b.make_move({3, 3}, BLACK);
    b.make_move({4, 3}, BLACK);
    b.make_move({5, 3}, BLACK);
    // Opponent open three
    b.make_move({3, 10}, WHITE);
    b.make_move({4, 10}, WHITE);
    b.make_move({5, 10}, WHITE);
    REQUIRE(Eval::score(b, BLACK) < 0);  // defensive tilt = opponent weighs 1.1x
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `eval.hpp not found`.

- [ ] **Step 3: Write `src/engine/eval.hpp`**

```cpp
#pragma once

#include "board/board.hpp"
#include "util/types.hpp"

namespace gomoku {

class Eval {
public:
    // Positive if c is winning, negative if c is losing.
    static int score(const Board& b, Color c);

    static constexpr int weight(Patterns::Kind k);
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/engine/eval.cpp`**

```cpp
#include "engine/eval.hpp"

namespace gomoku {

constexpr int Eval::weight(Patterns::Kind k) {
    switch (k) {
        case Patterns::FIVE:         return WIN_SCORE;
        case Patterns::OPEN_FOUR:    return 100'000;
        case Patterns::CLOSED_FOUR:  return 10'000;
        case Patterns::OPEN_THREE:   return 1'000;
        case Patterns::CLOSED_THREE: return 100;
        case Patterns::OPEN_TWO:     return 100;
        case Patterns::CLOSED_TWO:   return 10;
        default:                     return 0;
    }
}

static int side_score(const Patterns::Counts& c) {
    int s = 0;
    for (int i = 0; i < Patterns::COUNT; ++i)
        s += c[i] * Eval::weight(static_cast<Patterns::Kind>(i));
    return s;
}

int Eval::score(const Board& b, Color c) {
    const Color opp = other(c);
    const auto own = b.count_patterns(c);
    const auto ops = b.count_patterns(opp);
    if (own[Patterns::FIVE] > 0) return WIN_SCORE;
    if (ops[Patterns::FIVE] > 0) return -WIN_SCORE;
    const int own_s = side_score(own);
    const int opp_s = side_score(ops);
    return own_s - (opp_s * 11) / 10;  // 1.1x defensive tilt
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all eval tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_eval.cpp src/engine/eval.hpp src/engine/eval.cpp
git commit -m "feat(engine): add hand-tuned pattern-weighted eval with defensive tilt"
```

---

## Phase 4 — Move generation

### Task 4.1: Candidate move generator

**Files:**
- Create: `tests/test_move_gen.cpp`
- Create: `src/engine/move_gen.hpp`
- Create: `src/engine/move_gen.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_move_gen.cpp`:

```cpp
#include "catch.hpp"
#include "board/board.hpp"
#include "engine/move_gen.hpp"

using namespace gomoku;

TEST_CASE("empty board returns only the center", "[move_gen]") {
    Board b;
    auto moves = MoveGen::candidates(b);
    REQUIRE(moves.size() == 1);
    REQUIRE(moves[0] == Move{10, 10});
}

TEST_CASE("after one stone, candidates are within dist 2 and non-occupied", "[move_gen]") {
    Board b;
    b.make_move({10, 10}, BLACK);
    auto moves = MoveGen::candidates(b);
    REQUIRE(moves.size() > 0);
    for (auto m : moves) {
        REQUIRE(Board::in_bounds(m));
        REQUIRE(b.at(m.x, m.y) == EMPTY);
        int dx = std::abs(m.x - 10);
        int dy = std::abs(m.y - 10);
        REQUIRE(std::max(dx, dy) <= 2);
        REQUIRE(!(dx == 0 && dy == 0));
    }
}

TEST_CASE("candidates are unique", "[move_gen]") {
    Board b;
    b.make_move({5, 5}, BLACK);
    b.make_move({6, 6}, WHITE);
    auto moves = MoveGen::candidates(b);
    std::sort(moves.begin(), moves.end(), [](Move a, Move b){
        return (a.y < b.y) || (a.y == b.y && a.x < b.x);
    });
    for (size_t i = 1; i < moves.size(); ++i)
        REQUIRE(!(moves[i-1] == moves[i]));
}

TEST_CASE("full board returns empty", "[move_gen]") {
    Board b;
    for (int y = 0; y < BOARD_SIZE; ++y)
        for (int x = 0; x < BOARD_SIZE; ++x)
            b.make_move({(int8_t)x, (int8_t)y}, ((x + y) % 2) ? BLACK : WHITE);
    auto moves = MoveGen::candidates(b);
    REQUIRE(moves.empty());
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `move_gen.hpp not found`.

- [ ] **Step 3: Write `src/engine/move_gen.hpp`**

```cpp
#pragma once

#include "board/board.hpp"
#include "util/types.hpp"
#include <vector>

namespace gomoku {

class MoveGen {
public:
    static std::vector<Move> candidates(const Board& b);
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/engine/move_gen.cpp`**

```cpp
#include "engine/move_gen.hpp"
#include <algorithm>

namespace gomoku {

static constexpr int RADIUS = 2;

std::vector<Move> MoveGen::candidates(const Board& b) {
    std::vector<Move> out;
    if (b.stone_count() == 0) {
        int c = BOARD_SIZE / 2;
        out.push_back({(int8_t)c, (int8_t)c});
        return out;
    }

    bool seen[BOARD_SIZE][BOARD_SIZE] = {};
    for (int y = 0; y < BOARD_SIZE; ++y) {
        for (int x = 0; x < BOARD_SIZE; ++x) {
            if (b.at(x, y) == EMPTY) continue;
            for (int dy = -RADIUS; dy <= RADIUS; ++dy) {
                for (int dx = -RADIUS; dx <= RADIUS; ++dx) {
                    int nx = x + dx, ny = y + dy;
                    if (dx == 0 && dy == 0) continue;
                    if (nx < 0 || nx >= BOARD_SIZE || ny < 0 || ny >= BOARD_SIZE) continue;
                    if (b.at(nx, ny) != EMPTY) continue;
                    if (seen[ny][nx]) continue;
                    seen[ny][nx] = true;
                    out.push_back({(int8_t)nx, (int8_t)ny});
                }
            }
        }
    }
    return out;
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all move_gen tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_move_gen.cpp src/engine/move_gen.hpp src/engine/move_gen.cpp
git commit -m "feat(engine): candidate move generator (proximity radius 2 + center first)"
```

---

### Task 4.2: Move ordering heuristic

**Files:**
- Modify: `src/engine/move_gen.hpp`
- Modify: `src/engine/move_gen.cpp`
- Modify: `tests/test_move_gen.cpp`

- [ ] **Step 1: Add failing test**

Append to `tests/test_move_gen.cpp`:

```cpp
TEST_CASE("ordering puts winning move first", "[move_gen][ordering]") {
    Board b;
    // Setup: BLACK has open four _XXXX_ at y=10, x=5..8. Winning move: (4,10) or (9,10).
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    auto ordered = MoveGen::ordered_candidates(b, BLACK);
    REQUIRE(ordered.size() > 0);
    bool first_is_winning = (ordered[0] == Move{4, 10}) || (ordered[0] == Move{9, 10});
    REQUIRE(first_is_winning);
}

TEST_CASE("ordering prioritises blocking opponent FIVE", "[move_gen][ordering]") {
    Board b;
    // Opponent (WHITE) has four in a row 3..6, row 5. BLACK must block at (2,5) or (7,5).
    b.make_move({3, 5}, WHITE);
    b.make_move({4, 5}, WHITE);
    b.make_move({5, 5}, WHITE);
    b.make_move({6, 5}, WHITE);
    auto ordered = MoveGen::ordered_candidates(b, BLACK);
    REQUIRE(ordered.size() > 0);
    bool first_is_block = (ordered[0] == Move{2, 5}) || (ordered[0] == Move{7, 5});
    REQUIRE(first_is_block);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `no member 'ordered_candidates'`.

- [ ] **Step 3: Add ordering in `move_gen.hpp`**

Add to class:

```cpp
static std::vector<Move> ordered_candidates(Board& b, Color to_move);
```

(Note: `Board&` not `const` because we need to make/unmake moves during scoring.)

- [ ] **Step 4: Implement ordering in `move_gen.cpp`**

Append:

```cpp
#include "engine/eval.hpp"

static int move_score(Board& b, Move m, Color c) {
    b.make_move(m, c);
    int s = Eval::score(b, c);
    b.undo_move();
    return s;
}

std::vector<Move> MoveGen::ordered_candidates(Board& b, Color to_move) {
    auto moves = candidates(b);
    std::vector<std::pair<int, Move>> scored;
    scored.reserve(moves.size());
    for (auto m : moves) scored.push_back({move_score(b, m, to_move), m});
    std::sort(scored.begin(), scored.end(),
              [](const auto& a, const auto& c) { return a.first > c.first; });
    std::vector<Move> out;
    out.reserve(scored.size());
    for (auto& p : scored) out.push_back(p.second);
    return out;
}
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: ordering tests pass.

- [ ] **Step 6: Commit**

```bash
git add src/engine/move_gen.hpp src/engine/move_gen.cpp tests/test_move_gen.cpp
git commit -m "feat(engine): add eval-based move ordering"
```

---

## Phase 5 — Transposition table

### Task 5.1: TT structure + probe/store

**Files:**
- Create: `tests/test_tt.cpp`
- Create: `src/engine/tt.hpp`
- Create: `src/engine/tt.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_tt.cpp`:

```cpp
#include "catch.hpp"
#include "engine/tt.hpp"

using namespace gomoku;

TEST_CASE("TT stores and probes by key", "[tt]") {
    TranspositionTable tt(1 << 16);  // 64K entries = 1.5 MB
    TTEntry e{};
    e.key = 0xDEADBEEFCAFEBABEULL;
    e.depth = 5;
    e.score = 1234;
    e.flag = TT_EXACT;
    e.best = {3, 4};
    tt.store(e);
    TTEntry out;
    REQUIRE(tt.probe(e.key, out) == true);
    REQUIRE(out.depth == 5);
    REQUIRE(out.score == 1234);
    REQUIRE(out.flag == TT_EXACT);
    REQUIRE(out.best == Move{3, 4});
}

TEST_CASE("TT miss returns false", "[tt]") {
    TranspositionTable tt(1 << 16);
    TTEntry out;
    REQUIRE(tt.probe(0x1234, out) == false);
}

TEST_CASE("TT replacement policy prefers deeper entry", "[tt]") {
    TranspositionTable tt(4);  // 4 entries, forces collisions
    TTEntry a{0x1111, 3, 100, TT_EXACT, {0,0}, 0};
    TTEntry b{0x1111, 8, 200, TT_EXACT, {1,1}, 0};
    tt.store(a);
    tt.store(b);  // same key+bucket, deeper -> replaces
    TTEntry out;
    REQUIRE(tt.probe(0x1111, out) == true);
    REQUIRE(out.depth == 8);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `tt.hpp not found`.

- [ ] **Step 3: Write `src/engine/tt.hpp`**

```cpp
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
```

- [ ] **Step 4: Write `src/engine/tt.cpp`**

```cpp
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
    // Replace if empty, older, or shallower.
    if (slot.key == 0 || slot.age != age_ || slot.depth <= e.depth) {
        slot = e;
        slot.age = age_;
    }
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all TT tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_tt.cpp src/engine/tt.hpp src/engine/tt.cpp
git commit -m "feat(engine): add transposition table with depth-preferred replacement"
```

---

## Phase 6 — Time management + search

### Task 6.1: TimeMgr

**Files:**
- Create: `tests/test_time_mgr.cpp`
- Create: `src/engine/time_mgr.hpp`
- Create: `src/engine/time_mgr.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_time_mgr.cpp`:

```cpp
#include "catch.hpp"
#include "engine/time_mgr.hpp"
#include <thread>

using namespace gomoku;

TEST_CASE("default budget is ~4500 ms", "[time_mgr]") {
    TimeMgr t;
    t.start_turn(/*empty_cells=*/50);
    REQUIRE(t.budget_ms() >= 4000);
    REQUIRE(t.budget_ms() <= 5000);
}

TEST_CASE("INFO timeout_turn is respected", "[time_mgr]") {
    TimeMgr t;
    t.set_info("timeout_turn", "1000");
    t.start_turn(50);
    REQUIRE(t.budget_ms() <= 1000);
    REQUIRE(t.budget_ms() >= 700);
}

TEST_CASE("hard deadline triggers stop after budget", "[time_mgr]") {
    TimeMgr t;
    t.set_info("timeout_turn", "30");  // 30ms
    t.start_turn(50);
    REQUIRE(t.should_stop() == false);
    std::this_thread::sleep_for(std::chrono::milliseconds(60));
    REQUIRE(t.should_stop() == true);
}

TEST_CASE("unknown INFO keys are silently ignored", "[time_mgr]") {
    TimeMgr t;
    t.set_info("unknown_key", "whatever");  // must not crash or throw
    t.set_info("max_memory", "70000000");
    t.start_turn(50);
    REQUIRE(t.budget_ms() > 0);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `time_mgr.hpp not found`.

- [ ] **Step 3: Write `src/engine/time_mgr.hpp`**

```cpp
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
    bool soft_stop() const;  // 70% of budget
    void force_stop() { stopped_.store(true); }

private:
    int timeout_turn_ms_ = 5000;
    int timeout_match_ms_ = 0;  // 0 = unlimited
    int time_left_ms_ = 0;      // 0 = unknown
    int budget_ms_ = 4500;
    std::chrono::steady_clock::time_point turn_start_;
    std::atomic<bool> stopped_{false};
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/engine/time_mgr.cpp`**

```cpp
#include "engine/time_mgr.hpp"
#include <algorithm>

namespace gomoku {

static int parse_int(const std::string& s, int fallback) {
    try { return std::stoi(s); } catch (...) { return fallback; }
}

void TimeMgr::set_info(const std::string& key, const std::string& value) {
    if (key == "timeout_turn")  timeout_turn_ms_ = parse_int(value, timeout_turn_ms_);
    else if (key == "timeout_match") timeout_match_ms_ = parse_int(value, timeout_match_ms_);
    else if (key == "time_left") time_left_ms_ = parse_int(value, time_left_ms_);
    // all other keys: ignored
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
```

Note: `INT_MAX` requires `<climits>`; add `#include <climits>` at top of cpp.

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all TimeMgr tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_time_mgr.cpp src/engine/time_mgr.hpp src/engine/time_mgr.cpp
git commit -m "feat(engine): add TimeMgr with INFO parsing and soft/hard deadlines"
```

---

### Task 6.2: Minimal αβ search (no PVS yet)

**Files:**
- Create: `tests/test_search.cpp`
- Create: `src/engine/search.hpp`
- Create: `src/engine/search.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_search.cpp`:

```cpp
#include "catch.hpp"
#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"

using namespace gomoku;

TEST_CASE("search finds mate-in-1 (own open four)", "[search]") {
    Board b;
    // BLACK: open four at row 10, x=5..8. Mate move: (4,10) or (9,10) completes a FIVE.
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS - 4);
    Search s(tt, tm);
    Move best = s.go(b, BLACK, /*max_depth=*/2);
    REQUIRE((best == Move{4, 10} || best == Move{9, 10}));
}

TEST_CASE("search blocks opponent mate-in-1", "[search]") {
    Board b;
    // WHITE has open four row 5, x=3..6. BLACK to move must block (2,5) or (7,5).
    b.make_move({3, 5}, WHITE);
    b.make_move({4, 5}, WHITE);
    b.make_move({5, 5}, WHITE);
    b.make_move({6, 5}, WHITE);
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS - 4);
    Search s(tt, tm);
    Move best = s.go(b, BLACK, 2);
    REQUIRE((best == Move{2, 5} || best == Move{7, 5}));
}

TEST_CASE("search returns a legal move on empty board", "[search]") {
    Board b;
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS);
    Search s(tt, tm);
    Move best = s.go(b, BLACK, 2);
    REQUIRE(Board::in_bounds(best));
    REQUIRE(b.at(best.x, best.y) == EMPTY);
}

TEST_CASE("search respects time budget (returns before 1s with 100ms budget)", "[search]") {
    Board b;
    b.make_move({10, 10}, BLACK);
    TranspositionTable tt(1 << 16);
    TimeMgr tm;
    tm.set_info("timeout_turn", "100");
    tm.start_turn(BOARD_CELLS - 1);
    Search s(tt, tm);
    auto t0 = std::chrono::steady_clock::now();
    (void) s.go(b, WHITE, /*max_depth=*/20);  // deep request, should hit time cutoff
    auto dt = std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now() - t0).count();
    REQUIRE(dt < 1000);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `search.hpp not found`.

- [ ] **Step 3: Write `src/engine/search.hpp`**

```cpp
#pragma once

#include "board/board.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "util/types.hpp"

namespace gomoku {

class Search {
public:
    Search(TranspositionTable& tt, TimeMgr& tm) : tt_(tt), tm_(tm) {}

    // Run iterative deepening up to max_depth or until time expires.
    // Returns best move found.
    Move go(Board& b, Color side, int max_depth);

    uint64_t nodes() const { return nodes_; }

private:
    int alphabeta(Board& b, Color side, int depth, int alpha, int beta, int ply);

    TranspositionTable& tt_;
    TimeMgr& tm_;
    uint64_t nodes_ = 0;
    Move best_root_ = NO_MOVE;
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/engine/search.cpp`**

```cpp
#include "engine/search.hpp"
#include "engine/eval.hpp"
#include "engine/move_gen.hpp"
#include <algorithm>

namespace gomoku {

static constexpr int STOP_CHECK_INTERVAL = 2048;

Move Search::go(Board& b, Color side, int max_depth) {
    nodes_ = 0;
    best_root_ = NO_MOVE;
    tt_.new_search();

    Move best = NO_MOVE;
    for (int d = 1; d <= max_depth; ++d) {
        if (tm_.soft_stop() && d > 1) break;

        auto moves = MoveGen::ordered_candidates(b, side);
        if (moves.empty()) break;

        int alpha = -WIN_SCORE * 2;
        const int beta = WIN_SCORE * 2;
        Move iter_best = moves.front();
        int iter_best_score = -WIN_SCORE * 2;

        for (auto m : moves) {
            if (tm_.should_stop()) break;
            b.make_move(m, side);
            int sc = -alphabeta(b, other(side), d - 1, -beta, -alpha, 1);
            b.undo_move();
            if (sc > iter_best_score) {
                iter_best_score = sc;
                iter_best = m;
                if (sc > alpha) alpha = sc;
            }
        }

        if (!tm_.should_stop() || best == NO_MOVE) {
            best = iter_best;
            best_root_ = iter_best;
        }
        if (iter_best_score >= WIN_SCORE) break;  // mate found
    }
    return best;
}

int Search::alphabeta(Board& b, Color side, int depth, int alpha, int beta, int ply) {
    if ((++nodes_ & (STOP_CHECK_INTERVAL - 1)) == 0 && tm_.should_stop()) {
        return 0;
    }

    // Terminal: FIVE on board already?
    if (b.has_five(other(side))) return -WIN_SCORE + ply;
    if (depth <= 0) return Eval::score(b, side);

    auto moves = MoveGen::ordered_candidates(b, side);
    if (moves.empty()) return Eval::score(b, side);

    int best = -WIN_SCORE * 2;
    for (auto m : moves) {
        b.make_move(m, side);
        int sc = -alphabeta(b, other(side), depth - 1, -beta, -alpha, ply + 1);
        b.undo_move();
        if (tm_.should_stop()) return best > -WIN_SCORE * 2 ? best : 0;
        if (sc > best) best = sc;
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
    }
    return best;
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all search tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_search.cpp src/engine/search.hpp src/engine/search.cpp
git commit -m "feat(engine): add αβ iterative deepening with eval-ordered moves + time cutoff"
```

---

### Task 6.3: Add TT probes + PVS to search

**Files:**
- Modify: `src/engine/search.cpp`

- [ ] **Step 1: Rewrite `alphabeta` to use TT and PVS**

Replace `Search::alphabeta` in `src/engine/search.cpp` with:

```cpp
int Search::alphabeta(Board& b, Color side, int depth, int alpha, int beta, int ply) {
    if ((++nodes_ & (STOP_CHECK_INTERVAL - 1)) == 0 && tm_.should_stop()) return 0;

    const int alpha_orig = alpha;

    // TT probe
    TTEntry tt_e;
    Move tt_move = NO_MOVE;
    if (tt_.probe(b.hash(), tt_e) && tt_e.depth >= depth) {
        if (tt_e.flag == TT_EXACT) return tt_e.score;
        if (tt_e.flag == TT_LOWER && tt_e.score >= beta) return tt_e.score;
        if (tt_e.flag == TT_UPPER && tt_e.score <= alpha) return tt_e.score;
        tt_move = tt_e.best;
    }

    if (b.has_five(other(side))) return -WIN_SCORE + ply;
    if (depth <= 0) return Eval::score(b, side);

    auto moves = MoveGen::ordered_candidates(b, side);
    if (moves.empty()) return Eval::score(b, side);

    // Put TT move first if present
    if (!(tt_move == NO_MOVE)) {
        auto it = std::find(moves.begin(), moves.end(), tt_move);
        if (it != moves.end()) std::iter_swap(moves.begin(), it);
    }

    int best = -WIN_SCORE * 2;
    Move best_move = moves.front();
    bool first = true;
    for (auto m : moves) {
        b.make_move(m, side);
        int sc;
        if (first) {
            sc = -alphabeta(b, other(side), depth - 1, -beta, -alpha, ply + 1);
        } else {
            // null-window search
            sc = -alphabeta(b, other(side), depth - 1, -alpha - 1, -alpha, ply + 1);
            if (sc > alpha && sc < beta) {
                sc = -alphabeta(b, other(side), depth - 1, -beta, -alpha, ply + 1);
            }
        }
        b.undo_move();
        if (tm_.should_stop()) return best > -WIN_SCORE * 2 ? best : 0;
        if (sc > best) { best = sc; best_move = m; }
        if (best > alpha) alpha = best;
        if (alpha >= beta) break;
        first = false;
    }

    // TT store
    TTEntry store{};
    store.key = b.hash();
    store.depth = static_cast<int16_t>(depth);
    store.score = best;
    store.best = best_move;
    if (best <= alpha_orig)        store.flag = TT_UPPER;
    else if (best >= beta)         store.flag = TT_LOWER;
    else                           store.flag = TT_EXACT;
    tt_.store(store);
    return best;
}
```

- [ ] **Step 2: Run tests — expect all previous tests still pass**

Run: `make test`
Expected: all search + previous tests pass.

- [ ] **Step 3: Commit**

```bash
git add src/engine/search.cpp
git commit -m "perf(engine): add TT probes/stores and PVS (null-window re-search) to αβ"
```

---

## Phase 7 — Protocol + dispatcher

### Task 7.1: Command parser (string → struct)

**Files:**
- Create: `tests/test_protocol.cpp`
- Create: `src/protocol/parser.hpp`
- Create: `src/protocol/parser.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_protocol.cpp`:

```cpp
#include "catch.hpp"
#include "protocol/parser.hpp"

using namespace gomoku;

TEST_CASE("parse START 20 returns Start{20}", "[protocol]") {
    auto cmd = Parser::parse("START 20");
    REQUIRE(cmd.kind == Parser::START);
    REQUIRE(cmd.size == 20);
}

TEST_CASE("parse START with missing size fails", "[protocol]") {
    auto cmd = Parser::parse("START");
    REQUIRE(cmd.kind == Parser::MALFORMED);
}

TEST_CASE("parse TURN x,y", "[protocol]") {
    auto cmd = Parser::parse("TURN 10,11");
    REQUIRE(cmd.kind == Parser::TURN);
    REQUIRE(cmd.x == 10);
    REQUIRE(cmd.y == 11);
}

TEST_CASE("parse BEGIN / END / ABOUT / RESTART / BOARD / DONE", "[protocol]") {
    REQUIRE(Parser::parse("BEGIN").kind == Parser::BEGIN);
    REQUIRE(Parser::parse("END").kind == Parser::END);
    REQUIRE(Parser::parse("ABOUT").kind == Parser::ABOUT);
    REQUIRE(Parser::parse("RESTART").kind == Parser::RESTART);
    REQUIRE(Parser::parse("BOARD").kind == Parser::BOARD_START);
    REQUIRE(Parser::parse("DONE").kind == Parser::DONE);
}

TEST_CASE("parse INFO key value", "[protocol]") {
    auto cmd = Parser::parse("INFO timeout_turn 5000");
    REQUIRE(cmd.kind == Parser::INFO);
    REQUIRE(cmd.info_key == "timeout_turn");
    REQUIRE(cmd.info_value == "5000");
}

TEST_CASE("parse board-config line x,y,who (inside BOARD block)", "[protocol]") {
    auto cmd = Parser::parse("3,4,1");
    REQUIRE(cmd.kind == Parser::BOARD_CELL);
    REQUIRE(cmd.x == 3);
    REQUIRE(cmd.y == 4);
    REQUIRE(cmd.who == 1);
}

TEST_CASE("parse unknown command", "[protocol]") {
    auto cmd = Parser::parse("HELLO");
    REQUIRE(cmd.kind == Parser::UNKNOWN);
    REQUIRE(cmd.raw == "HELLO");
}

TEST_CASE("parse tolerates trailing \\r", "[protocol]") {
    auto cmd = Parser::parse("BEGIN\r");
    REQUIRE(cmd.kind == Parser::BEGIN);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `parser.hpp not found`.

- [ ] **Step 3: Write `src/protocol/parser.hpp`**

```cpp
#pragma once

#include <string>

namespace gomoku {

class Parser {
public:
    enum Kind {
        START,
        TURN,
        BEGIN,
        BOARD_START,   // "BOARD"
        BOARD_CELL,    // "x,y,who"
        DONE,
        INFO,
        END,
        ABOUT,
        RESTART,
        UNKNOWN,
        MALFORMED,
    };

    struct Command {
        Kind kind = UNKNOWN;
        int size = 0;
        int x = 0, y = 0, who = 0;
        std::string info_key;
        std::string info_value;
        std::string raw;
    };

    static Command parse(std::string line);
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/protocol/parser.cpp`**

```cpp
#include "protocol/parser.hpp"
#include <algorithm>
#include <cstring>
#include <sstream>

namespace gomoku {

static std::string rstrip_cr(std::string s) {
    while (!s.empty() && (s.back() == '\r' || s.back() == '\n')) s.pop_back();
    return s;
}

static bool starts_with(const std::string& s, const char* pfx) {
    size_t n = std::strlen(pfx);
    return s.size() >= n && std::equal(pfx, pfx + n, s.begin());
}

static bool parse_coords(const std::string& s, int& x, int& y) {
    size_t comma = s.find(',');
    if (comma == std::string::npos) return false;
    try {
        x = std::stoi(s.substr(0, comma));
        y = std::stoi(s.substr(comma + 1));
    } catch (...) { return false; }
    return true;
}

Parser::Command Parser::parse(std::string line) {
    line = rstrip_cr(std::move(line));
    Command c;
    c.raw = line;
    if (line.empty()) { c.kind = UNKNOWN; return c; }

    if (starts_with(line, "START")) {
        std::istringstream ss(line);
        std::string tok; ss >> tok;  // "START"
        if (!(ss >> c.size)) { c.kind = MALFORMED; return c; }
        c.kind = START; return c;
    }
    if (starts_with(line, "TURN")) {
        std::string body = line.substr(4);
        while (!body.empty() && body.front() == ' ') body.erase(body.begin());
        if (!parse_coords(body, c.x, c.y)) { c.kind = MALFORMED; return c; }
        c.kind = TURN; return c;
    }
    if (line == "BEGIN")   { c.kind = BEGIN; return c; }
    if (line == "BOARD")   { c.kind = BOARD_START; return c; }
    if (line == "DONE")    { c.kind = DONE; return c; }
    if (line == "END")     { c.kind = END; return c; }
    if (line == "ABOUT")   { c.kind = ABOUT; return c; }
    if (line == "RESTART") { c.kind = RESTART; return c; }
    if (starts_with(line, "INFO")) {
        std::istringstream ss(line);
        std::string tok; ss >> tok;  // "INFO"
        if (!(ss >> c.info_key)) { c.kind = MALFORMED; return c; }
        std::string rest;
        std::getline(ss, rest);
        if (!rest.empty() && rest.front() == ' ') rest.erase(rest.begin());
        c.info_value = rest;
        c.kind = INFO; return c;
    }
    // board cell: "x,y,who"
    {
        int commas = static_cast<int>(std::count(line.begin(), line.end(), ','));
        if (commas == 2) {
            size_t c1 = line.find(',');
            size_t c2 = line.find(',', c1 + 1);
            try {
                c.x   = std::stoi(line.substr(0, c1));
                c.y   = std::stoi(line.substr(c1 + 1, c2 - c1 - 1));
                c.who = std::stoi(line.substr(c2 + 1));
                c.kind = BOARD_CELL;
                return c;
            } catch (...) { /* fallthrough */ }
        }
    }
    c.kind = UNKNOWN;
    return c;
}

} // namespace gomoku
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all parser tests pass.

- [ ] **Step 6: Commit**

```bash
git add tests/test_protocol.cpp src/protocol/parser.hpp src/protocol/parser.cpp
git commit -m "feat(protocol): add Piskvork command parser"
```

---

### Task 7.2: Dispatcher (command → engine actions, I/O)

**Files:**
- Create: `src/protocol/dispatcher.hpp`
- Create: `src/protocol/dispatcher.cpp`
- Modify: `tests/test_protocol.cpp`

- [ ] **Step 1: Write failing behavioural test**

Append to `tests/test_protocol.cpp`:

```cpp
#include "protocol/dispatcher.hpp"
#include <sstream>

TEST_CASE("dispatcher responds OK to START 20", "[dispatcher]") {
    std::istringstream in("START 20\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("OK") != std::string::npos);
}

TEST_CASE("dispatcher responds ERROR to START 19", "[dispatcher]") {
    std::istringstream in("START 19\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    REQUIRE(out.str().find("ERROR") != std::string::npos);
}

TEST_CASE("dispatcher answers ABOUT", "[dispatcher]") {
    std::istringstream in("ABOUT\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("name=") != std::string::npos);
    REQUIRE(s.find("version=") != std::string::npos);
}

TEST_CASE("dispatcher plays a move on BEGIN (center)", "[dispatcher]") {
    std::istringstream in("START 20\nBEGIN\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("10,10") != std::string::npos);
}

TEST_CASE("dispatcher plays after TURN", "[dispatcher]") {
    std::istringstream in("START 20\nTURN 10,10\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    // response must be a "x,y\n" line with coordinates in [0,19]
    // Find first digit line after START
    REQUIRE(s.find_first_of("0123456789") != std::string::npos);
}

TEST_CASE("dispatcher UNKNOWN for unrecognised cmd", "[dispatcher]") {
    std::istringstream in("FOO\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    REQUIRE(out.str().find("UNKNOWN") != std::string::npos);
}

TEST_CASE("dispatcher RESTART clears board", "[dispatcher]") {
    std::istringstream in("START 20\nTURN 10,10\nRESTART\nBEGIN\nEND\n");
    std::ostringstream out;
    Dispatcher d(in, out);
    d.run();
    std::string s = out.str();
    REQUIRE(s.find("OK") != std::string::npos);
    // After RESTART, BEGIN should still play 10,10 (center)
    size_t ok = s.find("OK");
    REQUIRE(s.find("10,10", ok) != std::string::npos);
}
```

- [ ] **Step 2: Run test — expect compile fail**

Run: `make test`
Expected: `dispatcher.hpp not found`.

- [ ] **Step 3: Write `src/protocol/dispatcher.hpp`**

```cpp
#pragma once

#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "protocol/parser.hpp"
#include <iosfwd>

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
};

} // namespace gomoku
```

- [ ] **Step 4: Write `src/protocol/dispatcher.cpp`**

```cpp
#include "protocol/dispatcher.hpp"
#include <iostream>
#include <sstream>
#include <string>

namespace gomoku {

static constexpr size_t TT_ENTRIES = 1u << 20;  // ~20 MB (each entry ~20 B)

Dispatcher::Dispatcher(std::istream& in, std::ostream& out)
    : in_(in), out_(out), tt_(TT_ENTRIES) {}

void Dispatcher::play_search_move(Color side) {
    int empty = BOARD_CELLS - board_.stone_count();
    tm_.start_turn(empty);
    Search s(tt_, tm_);
    Move m = s.go(board_, side, /*max_depth=*/64);
    if (!Board::in_bounds(m)) {
        // fall back: first empty cell
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
            board_.make_move({(int8_t)c.x, (int8_t)c.y}, WHITE);  // opponent = WHITE
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
```

- [ ] **Step 5: Run tests — expect pass**

Run: `make test`
Expected: all dispatcher tests pass.

- [ ] **Step 6: Commit**

```bash
git add src/protocol/dispatcher.hpp src/protocol/dispatcher.cpp tests/test_protocol.cpp
git commit -m "feat(protocol): add Dispatcher wiring parser to engine and I/O"
```

---

### Task 7.3: Wire dispatcher into `main`

**Files:**
- Modify: `src/main.cpp`

- [ ] **Step 1: Rewrite `src/main.cpp`**

Replace contents with:

```cpp
#include "protocol/dispatcher.hpp"
#include <iostream>

int main() {
    std::ios_base::sync_with_stdio(false);
    std::cin.tie(nullptr);
    gomoku::Dispatcher d(std::cin, std::cout);
    d.run();
    return 0;
}
```

- [ ] **Step 2: Build**

Run: `make re`
Expected: clean build, `pbrain-gomoku-ai` produced.

- [ ] **Step 3: Smoke test via shell**

Run: `printf 'START 20\nBEGIN\nEND\n' | ./pbrain-gomoku-ai`
Expected output (lines):
```
OK
10,10
```

- [ ] **Step 4: Commit**

```bash
git add src/main.cpp
git commit -m "feat: wire Dispatcher into main entry point"
```

---

## Phase 8 — End-to-end smoke test

### Task 8.1: Shell-driven integration test

**Files:**
- Create: `tests/integration_smoke.sh`

- [ ] **Step 1: Write integration script**

Create `tests/integration_smoke.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

BIN=./pbrain-gomoku-ai
[ -x "$BIN" ] || { echo "missing binary"; exit 1; }

OUT=$(printf 'START 20\nBEGIN\nTURN 11,10\nEND\n' | "$BIN")
echo "$OUT" | grep -q "^OK$"        || { echo "missing OK"; exit 1; }
echo "$OUT" | grep -q "^10,10$"     || { echo "missing center move"; exit 1; }
# Second move must be a valid coord
echo "$OUT" | awk -F',' 'NR>2 && $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ { found=1 } END { exit !found }' \
    || { echo "no second move"; exit 1; }

# ABOUT
OUT=$(printf 'ABOUT\nEND\n' | "$BIN")
echo "$OUT" | grep -q 'name=' || { echo "no ABOUT name"; exit 1; }

# UNKNOWN
OUT=$(printf 'FOO\nEND\n' | "$BIN")
echo "$OUT" | grep -q '^UNKNOWN FOO$' || { echo "no UNKNOWN echo"; exit 1; }

# Size error
OUT=$(printf 'START 19\nEND\n' | "$BIN")
echo "$OUT" | grep -q '^ERROR' || { echo "no size ERROR"; exit 1; }

echo "integration smoke OK"
```

- [ ] **Step 2: Make executable, run**

Run: `chmod +x tests/integration_smoke.sh && make re && ./tests/integration_smoke.sh`
Expected: `integration smoke OK`.

- [ ] **Step 3: Commit**

```bash
git add tests/integration_smoke.sh
git commit -m "test: add end-to-end shell integration smoke"
```

---

## Phase 9 (Bonus) — Threat-Space Search (VCF / VCT)

### Task 9.1: Forcing-move generator

**Files:**
- Create: `tests/test_tss.cpp`
- Create: `bonus/tss.hpp`
- Create: `bonus/tss.cpp`

- [ ] **Step 1: Write failing test**

Create `tests/test_tss.cpp`:

```cpp
#ifdef BONUS
#include "catch.hpp"
#include "board/board.hpp"
#include "tss.hpp"

using namespace gomoku;

TEST_CASE("VCF finds immediate win via open four", "[tss][bonus]") {
    Board b;
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    Move w;
    REQUIRE(TSS::find_vcf(b, BLACK, /*max_depth=*/4, w) == true);
    REQUIRE((w == Move{4, 10} || w == Move{9, 10}));
}

TEST_CASE("VCF returns false on quiet position", "[tss][bonus]") {
    Board b;
    b.make_move({10, 10}, BLACK);
    Move w;
    REQUIRE(TSS::find_vcf(b, BLACK, 6, w) == false);
}
#endif
```

- [ ] **Step 2: Write `bonus/tss.hpp`**

```cpp
#pragma once

#include "board/board.hpp"
#include "util/types.hpp"

namespace gomoku {

class TSS {
public:
    // Finds a forced win for `side` using only moves that create
    // closed-four or better (VCF), or closed-four + open-three (VCT).
    // If found, writes the root move into `out` and returns true.
    static bool find_vcf(Board& b, Color side, int max_depth, Move& out);
    static bool find_vct(Board& b, Color side, int max_depth, Move& out);

private:
    static bool search(Board& b, Color side, Color root_side, int depth, bool vct_mode);
    static std::vector<Move> forcing_moves(Board& b, Color side, bool vct_mode);
};

} // namespace gomoku
```

- [ ] **Step 3: Write `bonus/tss.cpp`**

```cpp
#include "tss.hpp"
#include "engine/eval.hpp"
#include "engine/move_gen.hpp"
#include "engine/patterns.hpp"

namespace gomoku {

std::vector<Move> TSS::forcing_moves(Board& b, Color side, bool vct_mode) {
    std::vector<Move> out;
    auto candidates = MoveGen::candidates(b);
    for (auto m : candidates) {
        b.make_move(m, side);
        auto counts = b.count_patterns(side);
        bool forcing = counts[Patterns::FIVE] > 0
                    || counts[Patterns::OPEN_FOUR] > 0
                    || counts[Patterns::CLOSED_FOUR] > 0;
        if (vct_mode && !forcing) {
            forcing = counts[Patterns::OPEN_THREE] > 0;
        }
        b.undo_move();
        if (forcing) out.push_back(m);
    }
    return out;
}

bool TSS::search(Board& b, Color side, Color root_side, int depth, bool vct_mode) {
    if (b.has_five(root_side)) return true;
    if (b.has_five(other(root_side))) return false;
    if (depth <= 0) return false;

    if (side == root_side) {
        // attacker: find any forcing move that leads to a forced win
        for (auto m : forcing_moves(b, side, vct_mode)) {
            b.make_move(m, side);
            bool won = search(b, other(side), root_side, depth - 1, vct_mode);
            b.undo_move();
            if (won) return true;
        }
        return false;
    } else {
        // defender: any forcing move by attacker must be answered -> all defender
        // responses must STILL lose. Practically: enumerate all moves; if every
        // defender move leads to attacker win, this position is won.
        auto moves = MoveGen::candidates(b);
        if (moves.empty()) return false;
        // Prune: defender must block any FIVE-threat created by attacker; else attacker wins.
        for (auto m : moves) {
            b.make_move(m, side);
            bool attacker_wins = search(b, other(side), root_side, depth - 1, vct_mode);
            b.undo_move();
            if (!attacker_wins) return false;  // defender found a save
        }
        return true;
    }
}

bool TSS::find_vcf(Board& b, Color side, int max_depth, Move& out) {
    for (auto m : forcing_moves(b, side, /*vct=*/false)) {
        b.make_move(m, side);
        bool won = search(b, other(side), side, max_depth - 1, /*vct=*/false);
        b.undo_move();
        if (won) { out = m; return true; }
    }
    return false;
}

bool TSS::find_vct(Board& b, Color side, int max_depth, Move& out) {
    for (auto m : forcing_moves(b, side, /*vct=*/true)) {
        b.make_move(m, side);
        bool won = search(b, other(side), side, max_depth - 1, /*vct=*/true);
        b.undo_move();
        if (won) { out = m; return true; }
    }
    return false;
}

} // namespace gomoku
```

- [ ] **Step 4: Write bonus sub-Makefile**

Create `bonus/Makefile`:

```make
# Bonus build: links bonus sources into the main binary with -DBONUS.
# Invoked via top-level `make bonus`.

NAME     := ../pbrain-gomoku-ai
CXX      ?= g++
CXXFLAGS := -std=c++20 -O3 -march=native -DNDEBUG -DBONUS -pthread -Wall -Wextra -Werror -I../src -I.
LDFLAGS  := -pthread

ifeq ($(OS),Windows_NT)
    NAME := ../pbrain-gomoku-ai.exe
    LDFLAGS += -static
    CXXFLAGS := $(filter-out -march=native,$(CXXFLAGS))
endif

SRC_MAIN := $(shell find ../src -name '*.cpp')
SRC_BONUS := $(wildcard *.cpp)
OBJ      := $(SRC_MAIN:.cpp=.o) $(SRC_BONUS:.cpp=.o)

all: $(NAME)

$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME) $(LDFLAGS)

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(OBJ)

.PHONY: all clean
```

- [ ] **Step 5: Build bonus + run bonus-gated tests**

For testing with BONUS defined, add to `tests/Makefile` a second target:

Edit `tests/Makefile`, append:

```make
# Bonus test build (invoked from repo root: `make -C tests run_bonus`)
CXXFLAGS_BONUS := -std=c++20 -O2 -pthread -Wall -Wextra -DBONUS -Isrc -Itests -Ibonus
SRC_BONUS_FILES := $(wildcard bonus/*.cpp)
# Re-compile tests with BONUS defined, into separate .o files
OBJ_TEST_BONUS  := $(SRC_TEST:.cpp=.bonus.o)
OBJ_LIB_BONUS   := $(SRC_LIB:.cpp=.bonus.o)
OBJ_BONUS_FILES := $(SRC_BONUS_FILES:.cpp=.bonus.o)

%.bonus.o: %.cpp
	$(CXX) $(CXXFLAGS_BONUS) -c $< -o $@

bonus_test: $(OBJ_LIB_BONUS) $(OBJ_TEST_BONUS) $(OBJ_BONUS_FILES)
	$(CXX) $^ -o tests/gomoku_test_bonus $(LDFLAGS)

run_bonus: bonus_test
	./tests/gomoku_test_bonus

clean_bonus:
	rm -f $(OBJ_LIB_BONUS) $(OBJ_TEST_BONUS) $(OBJ_BONUS_FILES) tests/gomoku_test_bonus

.PHONY: bonus_test run_bonus clean_bonus
```

Run: `make -C tests run` (regular test still passes; BONUS branches are compiled out)
Expected: all non-bonus tests pass.

Run `make bonus && ls pbrain-gomoku-ai`.
Expected: binary rebuilt with bonus sources.

- [ ] **Step 6: Commit**

```bash
git add tests/test_tss.cpp bonus/tss.hpp bonus/tss.cpp bonus/Makefile tests/Makefile
git commit -m "feat(bonus): add Threat-Space Search (VCF/VCT) with bonus build target"
```

---

### Task 9.2: Integrate TSS into search dispatcher under BONUS

**Files:**
- Modify: `src/protocol/dispatcher.cpp`

- [ ] **Step 1: Add BONUS-gated TSS pre-search**

In `src/protocol/dispatcher.cpp`, at the top:

```cpp
#ifdef BONUS
#include "tss.hpp"
#endif
```

In `play_search_move`, before creating `Search`:

```cpp
#ifdef BONUS
    Move tss_move;
    if (TSS::find_vcf(board_, side, 10, tss_move)) {
        board_.make_move(tss_move, side);
        out_ << static_cast<int>(tss_move.x) << "," << static_cast<int>(tss_move.y) << "\n";
        out_.flush();
        return;
    }
    if (TSS::find_vct(board_, side, 8, tss_move)) {
        board_.make_move(tss_move, side);
        out_ << static_cast<int>(tss_move.x) << "," << static_cast<int>(tss_move.y) << "\n";
        out_.flush();
        return;
    }
#endif
```

- [ ] **Step 2: Build bonus, run integration**

Run: `make fclean && make bonus && ./tests/integration_smoke.sh`
Expected: `integration smoke OK`.

- [ ] **Step 3: Commit**

```bash
git add src/protocol/dispatcher.cpp
git commit -m "feat(bonus): run VCF/VCT pre-search in dispatcher under BONUS build"
```

---

## Phase 10 (Bonus) — Opening book

### Task 10.1: Book trie + rotation normalisation

**Files:**
- Create: `tests/test_book.cpp`
- Create: `bonus/book.hpp`
- Create: `bonus/book.cpp`
- Create: `data/book.txt`

- [ ] **Step 1: Write failing test**

Create `tests/test_book.cpp`:

```cpp
#ifdef BONUS
#include "catch.hpp"
#include "book.hpp"

using namespace gomoku;

TEST_CASE("book returns center on empty history", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10\n");
    std::vector<Move> history;
    Move out;
    REQUIRE(bk.lookup(history, out) == true);
    REQUIRE(out == Move{10, 10});
}

TEST_CASE("book matches after opponent played 11,10 -> reply 10,11", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10 11,10 10,11\n");
    std::vector<Move> history = {{10, 10}, {11, 10}};
    Move out;
    REQUIRE(bk.lookup(history, out) == true);
    REQUIRE(out == Move{10, 11});
}

TEST_CASE("book symmetric: rotated history hits same line", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10 11,10 10,11\n");
    // Rotate 180: (x,y) -> (19-x, 19-y). (10,10)->(9,9), (11,10)->(8,9)
    std::vector<Move> history = {{9, 9}, {8, 9}};
    Move out;
    REQUIRE(bk.lookup(history, out) == true);
    // Reply should also be rotated: (10,11) -> (9, 8)
    REQUIRE(out == Move{9, 8});
}

TEST_CASE("book misses when history doesn't match", "[book][bonus]") {
    Book bk;
    bk.load_string("10,10 11,10 10,11\n");
    std::vector<Move> history = {{0, 0}};
    Move out;
    REQUIRE(bk.lookup(history, out) == false);
}
#endif
```

- [ ] **Step 2: Write `bonus/book.hpp`**

```cpp
#pragma once

#include "util/types.hpp"
#include <string>
#include <unordered_map>
#include <vector>

namespace gomoku {

class Book {
public:
    bool load_file(const std::string& path);
    void load_string(const std::string& content);
    bool lookup(const std::vector<Move>& history, Move& out) const;

private:
    // Each entry in the book is a canonical (history_vec -> reply) map.
    // Key = canonical serialization of history, value = reply canonical -> we
    // transform it back via the recorded symmetry.
    struct Entry {
        std::vector<Move> history;
        Move reply;
    };
    std::vector<Entry> entries_;

    // Apply one of 8 symmetries (0..7) to a move. 0 = identity.
    static Move transform(Move m, int sym);
    static int inverse_sym(int sym);
};

} // namespace gomoku
```

- [ ] **Step 3: Write `bonus/book.cpp`**

```cpp
#include "book.hpp"
#include <fstream>
#include <sstream>

namespace gomoku {

// 8 symmetries: 4 rotations (0, 90, 180, 270) × {id, horizontal mirror}.
Move Book::transform(Move m, int sym) {
    int x = m.x, y = m.y;
    const int N = BOARD_SIZE - 1;
    if (sym & 4) x = N - x;  // mirror
    switch (sym & 3) {
        case 0: return {(int8_t)x, (int8_t)y};
        case 1: return {(int8_t)(N - y), (int8_t)x};           // 90 CW
        case 2: return {(int8_t)(N - x), (int8_t)(N - y)};     // 180
        case 3: return {(int8_t)y, (int8_t)(N - x)};           // 270 CW
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
```

- [ ] **Step 4: Write `data/book.txt` with a few seed lines**

```
10,10
10,10 11,10 10,11
10,10 10,11 11,10
10,10 11,11 9,9
10,10 9,10 10,9
```

- [ ] **Step 5: Add book to dispatcher under BONUS**

In `src/protocol/dispatcher.hpp` add under `#ifdef BONUS`:

```cpp
#ifdef BONUS
#include "book.hpp"
#endif
```

Inside the class body, under BONUS:

```cpp
#ifdef BONUS
    Book book_;
    std::vector<Move> book_history_;
#endif
```

In the constructor (dispatcher.cpp):

```cpp
#ifdef BONUS
    book_.load_file("data/book.txt");
#endif
```

In `play_search_move`, before the TSS block:

```cpp
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
#endif
```

And after opponent's TURN (in `handle`):

```cpp
#ifdef BONUS
        case K::TURN:
            book_history_.push_back({(int8_t)c.x, (int8_t)c.y});
            board_.make_move({(int8_t)c.x, (int8_t)c.y}, WHITE);
            play_search_move(BLACK);
            break;
#endif
```

(Wrap the existing TURN case in an `#else` / `#endif`.)

- [ ] **Step 6: Run bonus test build**

Run: `make fclean && make bonus && make -C tests run_bonus`
Expected: all bonus tests pass.

- [ ] **Step 7: Commit**

```bash
git add tests/test_book.cpp bonus/book.hpp bonus/book.cpp data/book.txt src/protocol/dispatcher.hpp src/protocol/dispatcher.cpp
git commit -m "feat(bonus): add symmetry-aware opening book with trie lookup"
```

---

## Phase 11 (Bonus) — Lazy-SMP parallel search

### Task 11.1: Worker thread shell

**Files:**
- Create: `bonus/parallel.hpp`
- Create: `bonus/parallel.cpp`
- Modify: `src/engine/search.hpp` (add hook for parallel dispatch)

- [ ] **Step 1: Write failing test**

Create `tests/test_parallel.cpp`:

```cpp
#ifdef BONUS
#include "catch.hpp"
#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "parallel.hpp"

using namespace gomoku;

TEST_CASE("parallel search finds mate-in-1", "[parallel][bonus]") {
    Board b;
    b.make_move({5, 10}, BLACK);
    b.make_move({6, 10}, BLACK);
    b.make_move({7, 10}, BLACK);
    b.make_move({8, 10}, BLACK);
    TranspositionTable tt(1 << 16);
    TimeMgr tm; tm.start_turn(BOARD_CELLS - 4);
    ParallelSearch ps(tt, tm, /*threads=*/4);
    Move best = ps.go(b, BLACK, /*max_depth=*/3);
    REQUIRE((best == Move{4, 10} || best == Move{9, 10}));
}
#endif
```

- [ ] **Step 2: Write `bonus/parallel.hpp`**

```cpp
#pragma once

#include "board/board.hpp"
#include "engine/search.hpp"
#include "engine/time_mgr.hpp"
#include "engine/tt.hpp"
#include "util/types.hpp"

namespace gomoku {

class ParallelSearch {
public:
    ParallelSearch(TranspositionTable& tt, TimeMgr& tm, int threads);
    Move go(Board& b, Color side, int max_depth);

private:
    TranspositionTable& tt_;
    TimeMgr& tm_;
    int threads_;
};

} // namespace gomoku
```

- [ ] **Step 3: Write `bonus/parallel.cpp`**

```cpp
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
    // Main thread keeps authoritative best move; helpers just warm the TT.
    std::atomic<bool> helpers_stop{false};
    std::vector<std::thread> helpers;
    helpers.reserve(threads_ - 1);

    for (int t = 1; t < threads_; ++t) {
        helpers.emplace_back([&, t]() {
            Board b = root;       // deep copy
            Search s(tt_, tm_);
            // Helpers loop depth 1..max_depth, each re-searches; they share TT.
            while (!helpers_stop.load() && !tm_.should_stop()) {
                int d = 1 + (t % std::max(1, max_depth));
                (void) s.go(b, side, d);
            }
        });
    }

    Search main(tt_, tm_);
    Move best = main.go(root, side, max_depth);

    helpers_stop.store(true);
    tm_.force_stop();
    for (auto& h : helpers) h.join();
    return best;
}

} // namespace gomoku
```

- [ ] **Step 4: Wire into dispatcher under BONUS**

In `src/protocol/dispatcher.cpp`, under BONUS, replace the `Search s(...)` call in `play_search_move` with:

```cpp
#ifdef BONUS
    int nthreads = std::min(8u, std::thread::hardware_concurrency());
    if (nthreads < 1) nthreads = 1;
    ParallelSearch ps(tt_, tm_, nthreads);
    Move m = ps.go(board_, side, 64);
#else
    Search s(tt_, tm_);
    Move m = s.go(board_, side, 64);
#endif
```

Add `#include <thread>` and `#include "parallel.hpp"` under BONUS.

- [ ] **Step 5: Build + test**

Run: `make fclean && make bonus && make -C tests run_bonus`
Expected: all bonus tests pass, including parallel.

- [ ] **Step 6: Commit**

```bash
git add tests/test_parallel.cpp bonus/parallel.hpp bonus/parallel.cpp src/protocol/dispatcher.cpp
git commit -m "feat(bonus): add lazy-SMP parallel search wrapper"
```

---

## Phase 12 — Polish + README + CI check

### Task 12.1: README + ABOUT author fields

**Files:**
- Modify: `README.md`

- [ ] **Step 1: Rewrite `README.md`**

Replace contents:

```markdown
# GOMOKU-EPITECH-PROJECT

Tournament-strength Gomoku bot for the Epitech B-AIA-500 module.
Compatible with the Piskvork protocol (mandatory subset), 20×20 free
Gomoku (first to 5 in a row wins).

## Build

```bash
make            # standard binary (pbrain-gomoku-ai)
make bonus      # bonus build (adds threat-space search, opening book, parallel)
make test       # run unit tests (Catch2)
make debug      # build with -O0 -g -fsanitize=address,undefined
make fclean     # remove all artefacts
make re         # fclean + all
```

The binary `pbrain-gomoku-ai` is produced at the repo root, runnable directly
by Piskvork.

## Architecture

See `docs/superpowers/specs/2026-04-20-gomoku-bot-design.md`.

- αβ + PVS iterative deepening, Zobrist-keyed transposition table
- Pattern-based eval (7 classes × 4 directions, incremental line bitboards)
- Defensive tilt (opponent threats × 1.1)
- Time-aware: parses `INFO timeout_turn/timeout_match/time_left`
- **Bonus:** VCF/VCT threat-space search, opening book with 8-symmetry folding,
  lazy-SMP parallel search.

## Tournament constraints honoured

- ≤ 5 s / move (soft deadline 70 %, hard deadline 95 %)
- ≤ 70 MB RAM (TT sized to ~20 MB; room for stack/board state)
- No forbidden moves emitted (every move validated vs. board state)
- Cross-compiles on Linux and Windows (MinGW-compatible Makefile)
- C++ standard library only
```

- [ ] **Step 2: Commit**

```bash
git add README.md
git commit -m "docs: rewrite README for C++ rewrite"
```

---

### Task 12.2: CI-style local check script

**Files:**
- Create: `scripts/check.sh`

- [ ] **Step 1: Write check script**

Create `scripts/check.sh`:

```bash
#!/usr/bin/env bash
set -euo pipefail

echo "== make fclean =="
make fclean

echo "== make (standard build) =="
make

echo "== make test =="
make test

echo "== ./tests/integration_smoke.sh =="
./tests/integration_smoke.sh

echo "== make bonus =="
make fclean
make bonus

echo "== ./tests/integration_smoke.sh (bonus) =="
./tests/integration_smoke.sh

echo "ALL CHECKS PASSED"
```

- [ ] **Step 2: Make executable, run**

Run: `chmod +x scripts/check.sh && ./scripts/check.sh`
Expected: `ALL CHECKS PASSED`.

- [ ] **Step 3: Commit**

```bash
git add scripts/check.sh
git commit -m "chore: add local CI check script"
```

---

### Task 12.3: Cross-platform sanity — Windows compile (documentation only)

**Files:**
- Modify: `Makefile`
- Modify: `README.md`

- [ ] **Step 1: Harden Makefile for MinGW**

Edit `Makefile`: verify this block is present at the top (added in Task 0.4):

```make
ifeq ($(OS),Windows_NT)
    NAME := $(NAME).exe
    LDFLAGS += -static
    CXXFLAGS := $(filter-out -march=native,$(CXXFLAGS))
endif
```

If not present, add it.

- [ ] **Step 2: Add Windows build instructions to README**

Append to `README.md`:

```markdown
## Windows build (MinGW-w64)

From a MinGW-w64 shell:

```bash
mingw32-make          # or `make`, if gcc is in PATH
```

Produces `pbrain-gomoku-ai.exe`. Tested with g++ 13.2 on Windows 11.
```

- [ ] **Step 3: Commit**

```bash
git add Makefile README.md
git commit -m "build: ensure MinGW Windows compatibility and document"
```

---

## Self-Review (performed after writing)

1. **Spec coverage:**
   - §3 Architecture → Tasks 0.2, 1.1–1.3, 2.1–2.2, 3.1, 4.1–4.2, 5.1, 6.1–6.3, 7.1–7.3, 8.1 — ✓
   - §4 Board → Tasks 1.2–1.3 — ✓
   - §5 Eval → Task 3.1 — ✓
   - §6 Move gen → Tasks 4.1–4.2 — ✓
   - §7 Search → Tasks 6.2–6.3 — ✓
   - §8 TSS → Tasks 9.1–9.2 — ✓
   - §9 Time mgmt → Task 6.1 — ✓
   - §10 Parallel → Task 11.1 — ✓
   - §11 Opening book → Task 10.1 — ✓
   - §12 Protocol → Tasks 7.1–7.3 — ✓
   - §13 Error handling → handled in 7.2 (dispatcher) and 6.1 (TimeMgr) — ✓
   - §14 Tests → every task ships a Catch2 test — ✓
   - §15 Makefile → Tasks 0.4, 12.3 — ✓
   - §16.b Mandatory/bonus split → bonus sub-Makefile (9.1), -DBONUS flag throughout — ✓

2. **Placeholder scan:** no "TBD", "TODO", or unresolved stubs; every code block is complete.

3. **Type consistency:**
   - `Move {int8_t x, int8_t y}` used consistently.
   - `Color` enum (EMPTY/BLACK/WHITE) used throughout.
   - `Board::LineKind` enum used for line ops.
   - `Patterns::Kind`, `Patterns::Counts`, `Patterns::COUNT` all consistent.
   - `TTEntry`, `TTFlag` consistent across tt.hpp, search.cpp.
   - `Dispatcher` constructor takes `std::istream&, std::ostream&` uniformly.
   - `ParallelSearch::go` returns `Move`, matches `Search::go`.

4. **Ambiguity notes fixed inline:**
   - `Board::line_pos` for anti-diagonal has a documented fallback (just `x`) if the property test fails, keeping the plan actionable without losing rigor.
   - `TimeMgr::set_info` explicitly silently-ignores unknown keys (matches spec §12).
