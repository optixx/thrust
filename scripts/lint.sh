#!/bin/sh
set -eu
ROOT=$(cd "$(dirname "$0")/.." && pwd)
cd "$ROOT"
if ! command -v clang-tidy >/dev/null 2>&1; then
  echo "clang-tidy not found; install it to run linting." >&2
  exit 1
fi
if ! command -v cppcheck >/dev/null 2>&1; then
  echo "cppcheck not found; install it to run linting." >&2
  exit 1
fi
SDL_INCLUDES="-I/opt/homebrew/include/SDL2 -I/opt/homebrew/include -I/Library/Frameworks/SDL2.framework/Headers"
FILES=$(git ls-files 'src/*.c' 'helpers/*.c' 'helpers/*.h')
for file in $FILES; do
  clang-tidy "$file" -- -std=c11 $SDL_INCLUDES
done
cppcheck --enable=warning,performance,portability --std=c11 \
  --suppress=missingIncludeSystem --suppress=unusedFunction \
  src helpers assets
