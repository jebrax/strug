# 📦 How to get it

Download the latest build for [macOS](https://github.com/jebrax/strug/releases/latest/download/strug-macos-arm64.tar.gz) or [Linux](https://github.com/jebrax/strug/releases/latest/download/strug-linux-x86_64.tar.gz). See [docs/DELIVERY.md](docs/DELIVERY.md) for how builds are released and delivered.

# 🧪 How to build

Right now only the macOS build is tested, building for other platforms might require some minor code modifications.

Clone this repo:

<pre>
git clone git@github.com:batonez/strug.git
</pre>

Then go to the repository root:

<pre>
cd strug
</pre>

Stay in this directory for the rest of the build process.

We have a bunch of dependencies living in submodules, so let's update those too:
<pre>
git submodule init
git submodule update
</pre>

## 🔧 Now let's build

`scripts/build-deps.sh` fetches the submodules and builds all the dependencies. The sections below describe them one by one.

Check if you are missing any of these tools:

<pre>
brew install autoconf automake libtool cmake
</pre>

[Autoconf](https://www.gnu.org/software/autoconf/) and [Automake](https://www.gnu.org/software/automake/) are used to build Freetype.
[Libtool](https://www.gnu.org/software/libtool/) is also used to build a part of our code and [cmake](https://cmake.org/) is used by almost everyone out there.

### Assimp

[Assimp](http://www.assimp.org/). We use it to load 3D models that were authored with external 3D modeling software.

To build it, execute these commands:

<pre>
pushd deps/assimp
  cmake CMakeLists.txt -DBUILD_SHARED_LIBS=OFF
  cmake --build .
popd
</pre>

For troubleshooting consult the [Build.md](https://github.com/batonez/assimp/blob/master/Build.md) file for detailed instructions on how to build Assimp.

### GLEW

[GLEW](http://glew.sourceforge.net/) stands for OpenGL extension wrangler library.

To build it, issue the following:

<pre>
pushd deps/glew
  pushd auto
    make
  popd
  make glew.lib.static
  git checkout doc/glew.html
popd
</pre>

Or see the `README.md` file in the root of our [GLEW repo](https://github.com/batonez/glew) for the build steps.

### GLFW

Glade uses [GLFW](https://www.glfw.org/), it's helpful if you don't wanna mess with platform specific windowing, events and input APIs Build: 

<pre>
pushd deps/glfw
  cmake -S . -B build
  pushd build
    make
  popd
popd
</pre>

The original build manual for GLFW is [here](https://www.glfw.org/docs/latest/compile.html).

### Zlib

[Zlib](http://zlib.net/) is. Some assets (3D models, textures and what not) use compression internally and we need to unpack them. Build the library:

<pre>
pushd deps/zlib
  ./configure --static
  make
  git checkout Makefile zconf.h
popd
</pre>

If something goes wrong consult the README file in the root of our [Zlib repo](https://github.com/batonez/zlib).

### Freetype

Renders fonts on the screen. [Freetype](http://freetype.org/) is exactly for this purpose. To build this library issue the following commands:

<pre>
pushd deps/freetype
  sh autogen.sh
  make setup ansi
  make
popd
</pre>

Again, original build instructions for Freetype are [here](https://github.com/batonez/freetype/blob/master/README.git) if you need them.

### Lightweight dependencies

There are some other dependencies but they are either single source or header only. They get compiled together with the app, so you don't need to build them separately. These dependencies are:

* [sha1](https://github.com/batonez/sha1) generates [SHA1 digests](https://en.wikipedia.org/wiki/SHA-1). We use it to store different kinds of objects in a hashmap.
* [PerlinNoise](https://github.com/batonez/PerlinNoise) is (as you have already guessed) a library for generating [Perlin gradient noises](https://en.wikipedia.org/wiki/Perlin_noise).
* [lodepng](https://github.com/batonez/lodepng) is a small set of functions to read [PNG images](https://en.wikipedia.org/wiki/Portable_Network_Graphics).

## 🔮 It works?!

We're almost there. Luckily, all the above dependencies are only needed to be built once. What we typically need to rebuild many times is our code which is split into two parts: the engine (**glade**) and the app (**strug**). To build both of them from your working directory run:

<pre>
make rr
</pre>

This will build **glade**, **strug** and run the application. Use this command whenever you modify the code under `deps/glade`.
If your modifications are only in the **strug** sources you can just say
<pre>
make
</pre>
to rebuild only the app and run it.

Finally, if you only need to run a previously built executable without recompiling:

<pre>
make run
</pre>

🔬*We are all set!*

## Glade API rename status (18 October 2026)

Strug's Glade calls, hook implementations, and overrides have been updated to
match the naming changes in the main `../glade` repository. These include:

- Controller methods: `Remap` → `remap`, `SetKeyMapper` → `setKeyMapper`,
  `UpdateKeyState` → `updateKeyState`, and `InputActionPress`/`InputActionRelease`
  → `inputActionPress`/`inputActionRelease`.
- State and object methods: `CreateEntityByTypeId` → `createEntityByTypeId`,
  `GetCurrentState` → `getCurrentState`, and `SetReplicasOutdated` →
  `setReplicasOutdated`.
- Stream methods: `Read`/`Write` → `read`/`write`, and `GetWrittenSize` →
  `getWrittenSize`.
- Other affected engine APIs, including `UpdateMeshVerticesFromGridIsoValues`
  → `updateMeshVerticesFromGridIsoValues` and free functions such as
  `getViewportSize` → `get_viewport_size` and `noise3D` → `noise3_d`.

No behaviour changes are intended. **Strug has not been rebuilt, run, or tested
after these renames; verification is still pending.** The successful beatemup
smoke test against main Glade does not verify Strug. The `deps/glade` checkout
was not updated as part of this work, so Strug needs the matching Glade API
changes before verification.

# Support status

Strug is unsupported from 2026-10-27. Future Glade changes do not preserve
compatibility with this application. See [GLADE_SUPPORT.md](GLADE_SUPPORT.md)
for the last supported Glade commit. The build instructions below are historical.
