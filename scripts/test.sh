#!/usr/bin/env bash
set -euo pipefail

CONFIG="${1:-Release}"

echo "==> Running tests (${CONFIG})"
ctest --test-dir build -C "${CONFIG}" --output-on-failure
