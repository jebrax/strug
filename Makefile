r: strug run

rr: glade strug run

run:
	./build/strug/strug

glade:
	cd deps/glade && ${MAKE} VENDOR=..

strug: set-env main-glfw.o ResourceManager.o Isosurface.o Cube.o Sphere.o Triangle.o CollisionTest.o Chunked.o Craft.o sha1.o assets builddir
	clang++ -O0 -g -o build/strug/strug -L${DEPS_PATH}/bullet3/build/src/BulletDynamics -L${DEPS_PATH}/zlib -L${DEPS_PATH}/assimp/lib -L${DEPS_PATH}/glew/lib/ -Ldeps/glade/build/glade -L${DEPS_PATH}/freetype/objs -L${DEPS_PATH}/glfw/build/src -L${DEPS_PATH}/bullet3/build/src/BulletCollision -lBulletCollision -L${DEPS_PATH}/bullet3/build/src/LinearMath -lLinearMath -lBulletDynamics -lz -lassimp -lglfw3 -lGLEW -lfreetype -lglade -framework OpenGL -framework AppKit -framework IOKit build/strug/main-glfw.o build/strug/ResourceManager.o build/strug/Isosurface.o build/strug/Cube.o build/strug/Sphere.o build/strug/Triangle.o build/strug/Craft.o build/strug/Chunked.o build/strug/CollisionTest.o build/sha1/sha1.o


######################

sha1.o: builddir
	clang -O0 -g -c ${DEPS_PATH}/sha1/sha1.c -o build/sha1/sha1.o

main.o: builddir
	clang -O0 -g -x objective-c -Ideps/glade/include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/main.mm -o build/strug/main.o

main-glfw.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -I${DEPS_PATH}/glew/include -I${DEPS_PATH}/glfw/include -Ideps/glade/include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/main-glfw.cpp -o build/strug/main-glfw.o

ResourceManager.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/ResourceManager.cpp -o build/strug/ResourceManager.o

Sphere.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -I${DEPS_PATH}/PerlinNoise/include -Ideps/glade/include -DGLADE_MACOS -c src/blocks/Sphere.cpp -o build/strug/Sphere.o

Frank.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -I${DEPS_PATH}/PerlinNoise/include -Ideps/glade/include -DGLADE_MACOS -c src/blocks/Frank.cpp -o build/strug/Frank.o

Triangle.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/blocks/Triangle.cpp -o build/strug/Triangle.o

Cube.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/blocks/Cube.cpp -o build/strug/Cube.o

Isosurface.o: builddir
	clang++ -std=c++17 -O0 -g -Iinclude -I${DEPS_PATH}/PerlinNoise/include -Ideps/glade/include -DGLADE_MACOS -c src/blocks/Isosurface.cpp -o build/strug/Isosurface.o

MazeTest.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/MazeTest.cpp -o build/strug/MazeTest.o

Minecraft.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/Minecraft.cpp -o build/strug/Minecraft.o

CubeTest.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/CubeTest.cpp -o build/strug/CubeTest.o

PhysicsTest.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/bullet3/src -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/PhysicsTest.cpp -o build/strug/PhysicsTest.o

CollisionTest.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/bullet3/src -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/CollisionTest.cpp -o build/strug/CollisionTest.o

MarchingCubes.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/MarchingCubes.cpp -o build/strug/MarchingCubes.o

Chunked.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/Chunked.cpp -o build/strug/Chunked.o

Craft.o: builddir
	clang++ -std=c++20 -O0 -g -I${DEPS_PATH}/glew/include -Iinclude -Ideps/glade/include -DGLADE_MACOS -c src/states/Craft.cpp -o build/strug/Craft.o

######################

assets: builddir
	mkdir -p build/strug/assets
	cp -R deps/glade/build/glade/assets/* build/strug/assets
	cp -R assets/* build/strug/assets

builddir: 
	mkdir -p build
	mkdir -p build/strug
	mkdir -p build/sha1

clean:
	rm -rf build

set-env:
ifndef DEPS_PATH
DEPS_PATH=deps
endif

