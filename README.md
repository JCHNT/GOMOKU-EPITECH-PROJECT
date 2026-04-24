# GOMOKU-EPITECH-PROJECT

Tournament-strength Gomoku bot for the Epitech B-AIA-500 module.
Compatible with the Piskvork protocol (mandatory subset), 20x20 free
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

- Alpha-beta + PVS iterative deepening, Zobrist-keyed transposition table
- Pattern-based eval (7 classes x 4 directions, incremental line bitboards)
- Defensive tilt (opponent threats x 1.1)
- Time-aware: parses `INFO timeout_turn/timeout_match/time_left`
- **Bonus:** VCF/VCT threat-space search, opening book with 8-symmetry folding,
  lazy-SMP parallel search.

## Tournament constraints honoured

- <= 5 s / move (soft deadline 70%, hard deadline 95%)
- <= 70 MB RAM (TT sized to ~20 MB; room for stack/board state)
- No forbidden moves emitted (every move validated vs. board state)
- Cross-compiles on Linux and Windows (MinGW-compatible Makefile)
- C++ standard library only

## Windows build (MinGW-w64)

From a MinGW-w64 shell:

```bash
mingw32-make          # or `make`, if gcc is in PATH
```

Produces `pbrain-gomoku-ai.exe`. Tested with g++ 13.2 on Windows 11.
