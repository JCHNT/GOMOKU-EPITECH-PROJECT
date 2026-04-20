# Gomoku Bot — Design Spec

**Project:** B-AIA-500 Gomoku (Epitech)
**Binary:** `pbrain-gomoku-ai`
**Language:** C++ (C++20)
**Date:** 2026-04-20

## 1. Goal

Build a tournament-strength Gomoku bot playing free 5-in-row on a 20×20 board,
compliant with the Piskvork protocol (mandatory commands only), compiling on
Linux and Windows via a single Makefile, using only the C++ standard library.

## 2. Constraints (from subject PDF)

- 20×20 goban, free Gomoku: first player to align 5 stones wins (H/V/diag).
- ≤ 5 s per move, ≤ 70 MB RAM.
- A forbidden move (occupied cell, out-of-bounds) = immediate defeat.
- Standard C++ libs only — no TensorFlow, scikit-learn, boost, etc.
- Must compile on both Linux (automated tests) and Windows (tournament) via `make`.
- Mandatory Piskvork protocol subset only.

## 3. Architecture

```
GOMOKU-EPITECH-PROJECT/
├── pbrain-gomoku-ai             # compiled binary (root, as per subject)
├── Makefile                     # all / re / clean / fclean / test / bonus
├── src/
│   ├── main.cpp                 # entry point, dispatches to Protocol loop
│   ├── protocol/
│   │   ├── parser.{hpp,cpp}     # tokenize + parse Piskvork commands
│   │   └── dispatcher.{hpp,cpp} # command → engine calls, I/O
│   ├── board/
│   │   ├── board.{hpp,cpp}      # state, make/unmake, line bitboards
│   │   └── zobrist.{hpp,cpp}    # hash tables + incremental hashing
│   ├── engine/
│   │   ├── patterns.{hpp,cpp}   # 7-class pattern detection per line
│   │   ├── eval.{hpp,cpp}       # position → int score
│   │   ├── move_gen.{hpp,cpp}   # candidate moves + ordering
│   │   ├── tt.{hpp,cpp}         # transposition table (lock-free)
│   │   ├── search.{hpp,cpp}     # αβ + PVS + iterative deepening
│   │   ├── tss.{hpp,cpp}        # VCF / VCT threat-space search
│   │   ├── parallel.{hpp,cpp}   # lazy SMP helper thread loop
│   │   ├── time_mgr.{hpp,cpp}   # deadline calc, stop flag
│   │   └── book.{hpp,cpp}       # opening book trie
│   └── util/
│       ├── log.{hpp,cpp}        # MESSAGE/DEBUG wrappers
│       └── types.hpp            # aliases, constants
├── data/
│   └── book.txt                 # hand-picked opening lines
├── bonus/                       # bonus-only sources (see §16.b)
│   ├── Makefile                 # sub-Makefile, builds bonus binary
│   ├── parallel.cpp             # lazy SMP worker
│   ├── book.cpp / book.hpp      # opening book trie
│   └── tss.cpp                  # threat-space search (VCF / VCT)
├── tests/
│   ├── catch.hpp                # vendored Catch2 single-header (test-only)
│   ├── test_board.cpp
│   ├── test_patterns.cpp
│   ├── test_eval.cpp
│   ├── test_move_gen.cpp
│   ├── test_tss.cpp
│   ├── test_search.cpp
│   ├── test_protocol.cpp
│   ├── test_book.cpp
│   └── test_time_mgr.cpp
└── docs/
    └── superpowers/specs/2026-04-20-gomoku-bot-design.md
```

Each module exposes a small header interface; internals are `.cpp`-private.

## 4. Board representation

- `uint8_t board[20][20]` — 0 empty, 1 black, 2 white.
- **Line bitboards (incremental):** 118 lines = 20 rows + 20 cols +
  39 diagonals (NE–SW) + 39 anti-diagonals (NW–SE). Each line stores two
  `uint32_t` masks (one per color, bit `i` set if that cell along the line
  holds the color's stone). Updated incrementally in `make_move` / `undo_move`.
  Pattern detection becomes a fixed set of bit-pattern matches against each
  affected line — O(constant) per move.
- **Zobrist hashing:** `uint64_t zob[2][400]` initialised once from `std::mt19937_64`
  with fixed seed. `hash ^= zob[color][idx]` on toggle; current hash stored on
  board, never recomputed.
- **Move history stack** for `undo_move` (stores previous hash + cell state).

## 5. Pattern classes and eval

Seven exclusive pattern classes detected per line:

| Class | Example | Meaning | Weight (own) |
|-------|---------|---------|--------------|
| FIVE       | `XXXXX`    | win                 | +∞ (WIN_SCORE = 10 000 000) |
| OPEN_FOUR  | `_XXXX_`   | unstoppable next    | 100 000 |
| CLOSED_FOUR| `OXXXX_`, `X_XXX`, `XX_XX` | forcing | 10 000 |
| OPEN_THREE | `_XXX_`, `_X_XX_`, `_XX_X_` | forcing | 1 000 |
| CLOSED_THREE | 3 stones with 1 side blocked | mild | 100 |
| OPEN_TWO   | `_XX_`, `_X_X_` | setup              | 100 |
| CLOSED_TWO | one-side-blocked 2 | weak setup      | 10 |

Score: `eval(pos) = sum(own_weights) − 1.1 × sum(opp_weights)` (defensive tilt).
`eval` terminates early if any FIVE is detected → returns signed WIN_SCORE.

Eval is **incremental-safe**: invoked only after `make_move`; recomputes only
the 4 lines through the played cell + caches in TT.

## 6. Move generation and ordering

- Candidate set = empty cells within Chebyshev distance 2 of any existing stone.
- Special case: empty board → center `(10, 10)`.
- Ordering (highest-first):
  1. TT best move (from transposition table probe)
  2. Moves creating own FIVE or blocking opp FIVE
  3. Moves creating own OPEN_FOUR / CLOSED_FOUR
  4. Killer moves (2 slots per ply)
  5. History heuristic score (`int history[400][400]`, incremented on cutoff)

## 7. Search — αβ + PVS + iterative deepening

- Iterative deepening from depth 2, `depth++` until stop flag set.
- **PVS (principal variation search):** first child searched with full `[α, β]`,
  subsequent children with null window `[α, α+1]`; on fail-high re-search
  with full window.
- **Transposition table:** open-addressing, 64 MB bucket array, each entry
  `{ key, depth, score, flag (EXACT/LOWER/UPPER), best_move, age }`.
  Replacement policy: prefer deeper or newer entry.
- **Aspiration windows** once depth ≥ 4: start with `[prev − 50, prev + 50]`,
  double on fail until full window.
- **Check-threat extension:** +1 ply when move creates OPEN_FOUR or blocks one.
- **No null-move pruning** (Gomoku is zugzwang-sensitive; passing is never free).
- **Quiescence search:** extends only on forcing moves
  (creates/blocks 4s and open 3s) until position is quiet; depth-capped at +8.

## 8. Threat-Space Search (TSS)

Before each iterative-deepening pass, run two narrow searches:

- **VCF (Victory by Continuous Fours):** DFS over moves that create a
  closed-four or better. Depth unbounded, branching ≤ 4. If found → line wins.
- **VCT (Victory by Continuous Threats):** VCF + OPEN_THREE moves, depth
  capped at 16. Wider branching but still narrow compared to full search.

If either returns a winning line → dispatcher outputs that root move directly,
skipping the main search.

## 9. Time management

- Parse `INFO timeout_turn <ms>`, `INFO timeout_match <ms>`,
  `INFO time_left <ms>`. Store on singleton `TimeMgr`.
- Per-move budget: `min(timeout_turn × 0.9, time_left / max(20, empty_cells))`.
- Defaults if absent: 4 500 ms per turn, unlimited match.
- Main thread tracks wall clock via `std::chrono::steady_clock`.
- **Soft deadline (70 % budget):** finish current depth, do not start a new one.
- **Hard deadline (95 % budget):** set atomic `stop_flag`; all search frames
  unwind on next check (every 2 048 nodes).
- Returns best move so far; never over-runs hard deadline.

## 10. Parallel search — Lazy SMP

- Worker count: `min(8, std::thread::hardware_concurrency())`.
- All workers search the **same root**, share the TT.
- TT access: lock-free XOR trick (store `key ^ data` alongside `data`; reader
  validates `data ^ stored_xor == key`). Avoids per-probe locking.
- Helper threads use hash-based move-order jitter to diverge exploration.
- Only the main thread increments iterative-deepening depth and owns the clock.
- All threads observe the same `stop_flag`.

## 11. Opening book

- `data/book.txt`: one entry per line = sequence of moves
  `x1,y1 x2,y2 … xn,yn response_x,response_y`.
- Loaded at startup into a trie keyed by normalized move sequence.
- **Normalization:** map each sequence to its canonical form under the 8
  symmetries of the square (4 rotations × mirror). Book match is symmetry-robust.
- While current game history matches a trie prefix → play the book response.
- Fall back to search once prefix no longer matches.
- Ship 100–200 hand-picked standard openings (book not required for correctness).

## 12. Protocol (Piskvork mandatory subset)

**Input commands:**

| Command | Action |
|---------|--------|
| `START <n>` | allocate n×n board. `OK` if n = 20 else `ERROR size not supported, only 20 accepted` |
| `TURN <x>,<y>` | opponent plays `(x,y)`, bot searches + outputs `x,y` |
| `BEGIN` | bot plays first (center), outputs `x,y` |
| `BOARD` … `<x>,<y>,<who>` … `DONE` | replay full state, then search + output `x,y` |
| `INFO <key> <value>` | store option (`timeout_turn`, `timeout_match`, `time_left`, `max_memory`, `game_type`, `rule`, `folder`); unknown keys silently ignored |
| `END` | `exit(0)` |
| `ABOUT` | `name="GOMOKU-EPITECH", version="1.0", author="Epitech", country="FR"` |
| `RESTART` | reset board + history, reply `OK` |
| unknown | `UNKNOWN <original>` |
| malformed | `ERROR <reason>` |

**Output discipline:** moves output as bare `x,y\n`. Diagnostics only via
`MESSAGE …` / `DEBUG …`. Never print anything else (no "Winner:", no
`print_board`, no banner).

## 13. Error handling

- Malformed input → `ERROR …` but continue loop (subject requires "forbidden
  move = defeat"; we never generate such moves, so this is defensive only).
- I/O failure on `std::cin` (EOF) → exit cleanly.
- Out-of-memory on TT alloc → retry with 16 MB, then 4 MB, then fail loudly.
- Threads join cleanly on `END`; `stop_flag` set first, then `join`.

## 14. Testing (Catch2, vendored)

`tests/catch.hpp` — Catch2 v2 single-header, test-only, not linked into the
bot binary.

| Suite | Covers |
|-------|--------|
| `test_board` | place / undo round-trip, Zobrist incremental = full recompute |
| `test_patterns` | each of 7 classes detected in all 4 directions, symmetric |
| `test_eval` | reflection symmetry, WIN > everything, empty board = 0 |
| `test_move_gen` | candidates respect proximity, first move = center, ordering stable |
| `test_tss` | known VCF positions solved; non-VCF returns none |
| `test_search` | mate-in-1, mate-in-3, blocks opp OPEN_FOUR, respects time |
| `test_protocol` | every command parsed correctly, errors on malformed |
| `test_book` | trie match + rotation/reflection normalization |
| `test_time_mgr` | budget respects `INFO`, hard deadline triggers stop_flag |

Tests invoked via `make test` → compiles `./tests/gomoku_test` → runs it.

## 15. Makefile

Single Makefile at repo root. Targets:

```make
NAME     = pbrain-gomoku-ai
CXX      = g++
CXXFLAGS = -std=c++20 -O3 -march=native -DNDEBUG -pthread -Wall -Wextra -Werror
LDFLAGS  = -pthread

ifeq ($(OS),Windows_NT)
    NAME := $(NAME).exe
    LDFLAGS += -static
endif

SRC      = $(shell find src -name '*.cpp')
OBJ      = $(SRC:.cpp=.o)

all: $(NAME)
$(NAME): $(OBJ)
	$(CXX) $(OBJ) -o $(NAME) $(LDFLAGS)
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@
clean:
	rm -f $(OBJ)
fclean: clean
	rm -f $(NAME)
re: fclean all
test:
	$(MAKE) -C tests && ./tests/gomoku_test
bonus: all
debug: CXXFLAGS = -std=c++20 -O0 -g -pthread -Wall -Wextra -fsanitize=address,undefined
debug: LDFLAGS += -fsanitize=address,undefined
debug: re
.PHONY: all re clean fclean test bonus debug
```

## 16.b Mandatory vs bonus split

- **Mandatory build (`make`):** single-threaded αβ + PVS + iterative deepening +
  TT + pattern eval + move ordering + time management + Piskvork protocol.
  Strong enough to beat low-to-medium bots.
- **Bonus build (`make bonus`):** adds lazy-SMP parallel search, threat-space
  search (VCF / VCT), and opening book. All three sources live under `bonus/`,
  compiled in with `-DBONUS` and linked into the same `pbrain-gomoku-ai`
  binary. Bonus sub-Makefile inside `bonus/` is invoked by the top-level
  `bonus` target per subject requirement.

## 16. Non-goals (YAGNI)

- No Renju forbidden-move enforcement (double-three, overline) — subject says
  simplified / free Gomoku.
- No GUI, no board printing to stdout.
- No neural network eval / self-play training (standard libs only).
- No network play — stdin/stdout only.
- No persistent learning between games.

## 17. Risk list

- **Windows cross-compile:** `-march=native` may not work under MinGW; fallback
  to `-O3` only if detected. `-pthread` should map to `-lpthread` equivalent.
- **TT contention on many cores:** lazy SMP scales sub-linearly past 8 threads.
- **Memory:** 64 MB TT + board state + stack frames ≈ 68 MB peak → within 70 MB
  cap but tight. Shrink TT to 32 MB if Windows arena bookkeeping pushes over.
- **Pattern detection correctness** is the single biggest correctness risk —
  the entire eval depends on it. Test suite must hit every pattern × direction.
- **Opening book:** optional; if loading fails bot falls back to center play.
