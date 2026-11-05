#!/usr/bin/env bash
# Checks a package the way a user would get it: unpacks the archive somewhere else and verifies its
# contents and that the executable doesn't need libraries users don't have. It doesn't start the game.
set -euo pipefail

ARCHIVE="$1"
WORK_DIR="$(mktemp -d)"
trap 'rm -rf "$WORK_DIR"' EXIT

tar -xzf "$ARCHIVE" -C "$WORK_DIR"
GAME="$WORK_DIR/strug/strug"

for file in "$WORK_DIR/strug/VERSION" "$WORK_DIR/strug/assets/shaders" "$WORK_DIR/strug/assets/non-pbr" "$WORK_DIR/strug/licenses"; do
  [ -e "$file" ] || { echo "FAIL: $file is missing from the package" >&2; exit 1; }
done

[ -x "$GAME" ] || { echo "FAIL: $GAME is missing or not executable" >&2; exit 1; }

if [ "$(uname -s)" = "Darwin" ]; then
  codesign --verify "$GAME"
  if otool -L "$GAME" | tail -n +2 | grep -vE '^\s*(/usr/lib/|/System/Library/)'; then
    echo "FAIL: the executable links against libraries that users don't have" >&2
    exit 1
  fi
else
  if ldd "$GAME" | grep -E 'not found|libc\+\+'; then
    echo "FAIL: the executable links against libraries that users don't have" >&2
    exit 1
  fi
fi

echo "PASS: $(basename "$ARCHIVE") version $(cat "$WORK_DIR/strug/VERSION")"
