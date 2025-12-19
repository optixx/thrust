#!/bin/sh
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
if ! command -v clang-format >/dev/null 2>&1; then
  echo "clang-format not found; install it to run formatting." >&2
  exit 1
fi
FILES=$(git ls-files '*.c' '*.h' '*.cpp' '*.hpp' | tr '\n' ' ')
if [ -z "$FILES" ]; then
  echo "No source files to format." >&2
  exit 1
fi
clang-format -i $FILES
