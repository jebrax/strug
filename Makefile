r: strug run

rr: glade strug run

run:
	./build/strug/strug

glade:
	cd ../glade && $(MAKE)

strug: main-glfw.o ResourceManager.o Frank.o Cube.o Sphere.o Isosurface.o MarchingCubes.o sha1.o builddir
	clang++ -mmacos-version-min=11.6 -O0 -g -o build/strug/strug -L../../vendor/libccd/src -L../../vendor/fcl/build/lib -L../../vendor/zlib-1.2.11 -L../../vendor/assimp/lib -L../../vendor/glew/lib/ -L../glade/build/glade -L../../vendor/freetype-2.10.1/objs/.libs/ -L../../vendor/glfw/build/src -lccd -lfcl -lz -lassimp -lglfw3 -lGLEW -lfreetype -lglade -framework OpenGL -framework AppKit -framework IOKit build/strug/main-glfw.o build/strug/ResourceManager.o build/strug/Sphere.o build/strug/Frank.o build/strug/Cube.o build/strug/Isosurface.o build/strug/MazeTest.o build/strug/Minecraft.o build/strug/MarchingCubes.o build/strug/CubeTest.o build/sha1/sha1.o


######################

sha1.o: builddir
	clang -O0 -g -c ../../vendor/sha1/sha1.c -o build/sha1/sha1.o

main.o: builddir
	clang -O0 -g -x objective-c -I ../glade/include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/main.mm -o build/strug/main.o

main-glfw.o: builddir
	clang++ -O0 -g -I include -I../../vendor/glew/include -I../../vendor/glfw/include -I ../glade/include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/main-glfw.cpp -o build/strug/main-glfw.o

ResourceManager.o: builddir
	clang++ -O0 -g -I include -I ../glade/include -DGLADE_MACOS -c src/ResourceManager.cpp -o build/strug/ResourceManager.o

Sphere.o: builddir
	clang++ -std=c++17 -O0 -g -I include -I../../vendor/PerlinNoise/include -I../glade/include -DGLADE_MACOS -c src/blocks/Sphere.cpp -o build/strug/Sphere.o

Frank.o: builddir
	clang++ -std=c++17 -O0 -g -I include -I../../vendor/PerlinNoise/include -I ../glade/include -DGLADE_MACOS -c src/blocks/Frank.cpp -o build/strug/Frank.o

Cube.o: builddir
	clang++ -std=c++17 -O0 -g -I include -I ../glade/include -DGLADE_MACOS -c src/blocks/Cube.cpp -o build/strug/Cube.o

Isosurface.o: builddir
	clang++ -std=c++17 -O0 -g -I include -I../../vendor/PerlinNoise/include -I ../glade/include -DGLADE_MACOS -c src/blocks/Isosurface.cpp -o build/strug/Isosurface.o

MazeTest.o: builddir
	clang++ -std=c++20 -O0 -g -I../../vendor/libccd/src -I../../vendor/eigen -I../../vendor/fcl/include -I../../vendor/fcl/build/include -I../../vendor/glew/include -I include -I ../glade/include -DGLADE_MACOS -c src/states/MazeTest.cpp -o build/strug/MazeTest.o

Minecraft.o: builddir
	clang++ -std=c++20 -O0 -g -I../../vendor/libccd/src -I../../vendor/PerlinNoise/include -I../../vendor/eigen -I../../vendor/fcl/include -I../../vendor/fcl/build/include -I../../vendor/glew/include -I include -I ../glade/include -DGLADE_MACOS -c src/states/Minecraft.cpp -o build/strug/Minecraft.o

CubeTest.o: builddir
	clang++ -std=c++20 -O0 -g -I../../vendor/libccd/src -I../../vendor/PerlinNoise/include -I../../vendor/eigen -I../../vendor/fcl/include -I../../vendor/fcl/build/include -I../../vendor/glew/include -I include -I ../glade/include -DGLADE_MACOS -c src/states/CubeTest.cpp -o build/strug/CubeTest.o

MarchingCubes.o: builddir
	clang++ -std=c++20 -O0 -g -I../../vendor/libccd/src -I../../vendor/PerlinNoise/include -I../../vendor/eigen -I../../vendor/fcl/include -I../../vendor/fcl/build/include -I../../vendor/glew/include -I include -I ../glade/include -DGLADE_MACOS -c src/states/MarchingCubes.cpp -o build/strug/MarchingCubes.o

######################

builddir: 
	mkdir -p build
	mkdir -p build/strug
	mkdir -p build/sha1

clean:
	rm -rf build
