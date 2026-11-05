#!/usr/bin/env bash
# Builds an optimized game from clean and packs it into dist/strug-<platform>.tar.gz. The file name
# has no version in it so that "latest" download links keep working, see docs/DELIVERY.md.
# The version comes from STRUG_VERSION, or from git if that's not set.
set -euo pipefail

cd "$(dirname "$0")/.."

case "$(uname -s)-$(uname -m)" in
  Darwin-arm64) PLATFORM=macos-arm64 ;;
  Linux-x86_64) PLATFORM=linux-x86_64 ;;
  *) echo "Unsupported platform: $(uname -s)-$(uname -m)" >&2; exit 1 ;;
esac

VERSION="${STRUG_VERSION:-$(git describe --tags --always --dirty | sed 's/^v//')}"
STAGE=dist/stage/strug
ARCHIVE="dist/strug-$PLATFORM.tar.gz"

[ "$PLATFORM" != macos-arm64 ] || export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-12.0}"

make clean
make RELEASE=1

rm -rf dist/stage "$ARCHIVE"
mkdir -p "$STAGE/licenses"
cp build/strug/strug "$STAGE/"
cp -R build/strug/assets "$STAGE/"
echo "$VERSION" > "$STAGE/VERSION"

if [ "$PLATFORM" = macos-arm64 ]; then
  strip -x "$STAGE/strug"
  # Stripping invalidates the signature, and arm64 macOS refuses to run binaries that don't have one
  codesign --force --sign - "$STAGE/strug"
else
  strip "$STAGE/strug"
fi

# Copyright notices of everything linked into the executable
for license in assimp/LICENSE enet/LICENSE freetype/docs/FTL.TXT glew/LICENSE.txt glfw/LICENSE.md \
    imgui/LICENSE.txt libccd/BSD-LICENSE lodepng/LICENSE PerlinNoise/LICENSE rapidjson/license.txt \
    zlib/README; do
  cp "deps/$license" "$STAGE/licenses/${license%%/*}-$(basename "$license")"
done

tar -czf "$ARCHIVE" -C dist/stage strug
rm -rf dist/stage

echo "Packaged version $VERSION: $ARCHIVE ($(du -h "$ARCHIVE" | cut -f1))"
