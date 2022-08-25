# 🧪 How to start doing the cool stuff?

For starters, get yourself a computer. With macOS. Becuase this pile of code won't compile easily elsewhere :)

Got it? Ok, now clone this repo:

<pre>
git clone git@github.com:batonez/strug.git
</pre>

Then go to the repository root:

<pre>
cd strug
</pre>

And stay in this directory for the rest of the build process (I warned you!).

We have a bunch of dependencies living in submodules, so let's update those too:
<pre>
git submodule update
</pre>

## 🔧 Now let's build build build!

Check if you are missing any of these good old tools (I'm sure you don't):

<pre>
brew install autoconf automake libtool cmake
</pre>

[Autoconf](https://www.gnu.org/software/autoconf/) and [Automake](https://www.gnu.org/software/automake/) are used to build Freetype.
[Libtool](https://www.gnu.org/software/libtool/) is also used to build a part of our code and [cmake](https://cmake.org/) is used by almost everyone out there.

### Assimp

[Assimp](http://www.assimp.org/) stands for the asset import library and not what you thought. We use it to load 3D models that were authored with external 3D modeling software.

To build it, execute this stuff:

<pre>
pushd deps/assimp
  cmake CMakeLists.txt -DBUILD_SHARED_LIBS=OFF
  cmake --build .
popd
</pre>

For troubleshooting consult the [Build.md](https://github.com/batonez/assimp/blob/master/Build.md) file for detailed instructions on how to build Assimp.

### GLEW

[GLEW](http://glew.sourceforge.net/) stands for OpenGL extension wrangler library. Without it we'd have to spend weeks writing boilerplate code that queries the OS for available OpenGL functions. Praise the gods other people do boring stuff, not us. 

To build it, issue the following:

<pre>
pushd deps/glew
  pushd auto
    make
  popd
  make glew.lib.static
popd
</pre>

Or see the `README.md` file in the root of our [GLEW repo](https://github.com/batonez/glew) for the build steps.

### GLFW

Not as boring as what GLEW does, but very helpful if you don't wanna mess with platform specific windowing, events and input APIs. I used to do that manually on Windows, but then I <strike>got an arrow in the knee</strike> switched to macOS and realised I can stay cross platform with [GLFW](https://www.glfw.org/) at no additional cost! (The project still lost it's compilability on Windows due to my lazyness, but that's another story). Nuff said: 

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

Everyone knows what [this](http://zlib.net/) is. Some assets (3D models, textures and what not) use compression internally and we need to unpack that. Let's build the library:

<pre>
pushd deps/zlib
  ./configure --static
  make
popd
</pre>

If something goes wrong consult the README file in the root of our [Zlib repo](https://github.com/batonez/zlib).

### Freetype

We need to render fonts on the screen right? It's a tricky thing to do when all you have is a GPU. That's why the world is grateful to the dudes who maintain [Freetype](http://freetype.org/) exactly for this purpose. To build this magic library issue the following:

<pre>
pushd deps/freetype
  sh autogen.sh
  make setup ansi
  make
popd
</pre>

Again, original build instructions for Freetype are [here](https://github.com/batonez/freetype/blob/master/README.git) if you need them.

### Lightweight dependencies

There are some other dependencies but they are either single source or header only. They get compiled together with the app, so you don't need to build them separately. If you wonder what these dependencies are:

* [sha1](https://github.com/batonez/sha1) generates [SHA1 digests](https://en.wikipedia.org/wiki/SHA-1). We use it to store different kinds of objects in a hashmap.
* [PerlinNoise](https://github.com/batonez/PerlinNoise) is (as you have already guessed) a library for generating [Perlin gradient noises](https://en.wikipedia.org/wiki/Perlin_noise).
* [lodepng](https://github.com/batonez/lodepng) is a small set of functions to read [PNG images](https://en.wikipedia.org/wiki/Portable_Network_Graphics).

## 🔮 It works?!

Phew! That's it. Luckily, all the above dependencies are only needed to be built once. What we typically need to rebuild many times is our code which is split into two parts: the engine (**glade**) and the app (**strug**). To build both of them from your working directory run:

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

🔬*We are all set for experiments!*

