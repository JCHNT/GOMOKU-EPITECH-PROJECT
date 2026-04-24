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
