#!/usr/bin/env bash
set -euo pipefail

BIN=./pbrain-gomoku-ai
[ -x "$BIN" ] || { echo "missing binary"; exit 1; }

OUT=$(printf 'START 20\nBEGIN\nTURN 11,10\nEND\n' | "$BIN")
echo "$OUT" | grep -q "^OK$"        || { echo "missing OK"; exit 1; }
echo "$OUT" | grep -q "^10,10$"     || { echo "missing center move"; exit 1; }
echo "$OUT" | awk -F',' 'NR>2 && $1 ~ /^[0-9]+$/ && $2 ~ /^[0-9]+$/ { found=1 } END { exit !found }' \
    || { echo "no second move"; exit 1; }

OUT=$(printf 'ABOUT\nEND\n' | "$BIN")
echo "$OUT" | grep -q 'name=' || { echo "no ABOUT name"; exit 1; }

OUT=$(printf 'FOO\nEND\n' | "$BIN")
echo "$OUT" | grep -q '^UNKNOWN FOO$' || { echo "no UNKNOWN echo"; exit 1; }

OUT=$(printf 'START 19\nEND\n' | "$BIN")
echo "$OUT" | grep -q '^ERROR' || { echo "no size ERROR"; exit 1; }

echo "integration smoke OK"
