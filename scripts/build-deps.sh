#!/usr/bin/env bash
# Fetches the pinned submodules and builds the native dependencies in place, this only needs to
# happen once per checkout. A dependency whose library is already there is skipped, delete the
# library to force a rebuild. Requirements are listed in docs/DELIVERY.md.
set -euo pipefail

cd "$(dirname "$0")/.."

git submodule update --init --depth 1 --jobs 4

export CC=clang CXX=clang++
JOBS=$(getconf _NPROCESSORS_ONLN)
CMAKE_ARGS=(-DCMAKE_BUILD_TYPE=Release -DCMAKE_POLICY_VERSION_MINIMUM=3.5)
ZLIB_CFLAGS="-O3"
CXX_STDLIB_FLAG=""

if [ "$(uname -s)" = "Darwin" ]; then
  export MACOSX_DEPLOYMENT_TARGET="${MACOSX_DEPLOYMENT_TARGET:-12.0}"
  # zlib 1.2.12 otherwise defines fdopen as a macro, which breaks recent macOS SDK headers
  ZLIB_CFLAGS="$ZLIB_CFLAGS -Dfdopen=fdopen"
else
  CXX_STDLIB_FLAG="-stdlib=libc++"
  CMAKE_ARGS+=(-DCMAKE_CXX_FLAGS="$CXX_STDLIB_FLAG")
fi

# GLEW generates its sources from these registries and otherwise takes their latest commits, which
# no longer produce compilable code for our GLEW. These are their states on GLEW's pinned commit date.
OPENGL_REGISTRY_COMMIT=11d7b7baebfc2b58116670cd17266f9c6a0d760b
EGL_REGISTRY_COMMIT=7dea2ed79187cd13f76183c4b9100159b9e3e071
GLFIXES_COMMIT=b63c8d3e676097d610efda64870cbe6f10543bd3

build_zlib() {
  ( cd deps/zlib
    CFLAGS="$ZLIB_CFLAGS" ./configure --static
    make -j"$JOBS" libz.a
    git checkout Makefile zconf.h )
}

build_glfw() {
  cmake -S deps/glfw -B deps/glfw/build "${CMAKE_ARGS[@]}" -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF -DGLFW_BUILD_WAYLAND=OFF
  cmake --build deps/glfw/build -j"$JOBS"
}

# All our models are .obj, the rest of the importers would only add build time and download size.
# Assimp always builds with -Werror, -w keeps warnings of newer compilers from failing the build.
build_assimp() {
  ( cd deps/assimp
    cmake CMakeLists.txt "${CMAKE_ARGS[@]}" -DCMAKE_CXX_FLAGS="-w $CXX_STDLIB_FLAG" -DBUILD_SHARED_LIBS=OFF -DASSIMP_BUILD_TESTS=OFF -DASSIMP_BUILD_ASSIMP_TOOLS=OFF \
      -DASSIMP_NO_EXPORT=ON -DASSIMP_BUILD_ALL_IMPORTERS_BY_DEFAULT=OFF -DASSIMP_BUILD_OBJ_IMPORTER=ON
    cmake --build . -j"$JOBS" )
}

build_freetype() {
  ( cd deps/freetype
    sh autogen.sh
    make setup ansi
    make -j"$JOBS" )
}

build_enet() {
  ( cd deps/enet
    autoreconf -vfi
    ./configure --disable-shared
    make -j"$JOBS" )
}

build_libccd() {
  make -C deps/libccd/src -j"$JOBS"
}

fetch_glew_registry() {
  [ -e "deps/glew/auto/$1/.dummy" ] && return
  git init -q "deps/glew/auto/$1"
  git -C "deps/glew/auto/$1" fetch -q --depth 1 "$2" "$3"
  git -C "deps/glew/auto/$1" checkout -q FETCH_HEAD
  touch "deps/glew/auto/$1/.dummy"
}

build_glew() {
  fetch_glew_registry OpenGL-Registry https://github.com/KhronosGroup/OpenGL-Registry.git "$OPENGL_REGISTRY_COMMIT"
  fetch_glew_registry EGL-Registry https://github.com/KhronosGroup/EGL-Registry.git "$EGL_REGISTRY_COMMIT"
  fetch_glew_registry glfixes https://github.com/nigels-com/glfixes "$GLFIXES_COMMIT"
  ( cd deps/glew
    make -C auto PYTHON=python3
    make glew.lib.static -j"$JOBS"
    git checkout doc/glew.html )
}

build_unless_exists() {
  if [ -e "$2" ]; then
    echo "=== $1: $2 exists, skipping"
  else
    echo "=== $1: building"
    "build_$1"
  fi
}

build_unless_exists zlib deps/zlib/libz.a
build_unless_exists glfw deps/glfw/build/src/libglfw3.a
build_unless_exists assimp deps/assimp/lib/libassimp.a
build_unless_exists freetype deps/freetype/objs/libfreetype.a
build_unless_exists enet deps/enet/.libs/libenet.a
build_unless_exists libccd deps/libccd/src/libccd.a
build_unless_exists glew deps/glew/lib/libGLEW.a
