ifndef DEPS_PATH
DEPS_PATH=deps
endif

ifndef STRUG_BUILD_DIR
STRUG_BUILD_DIR=build/strug
endif

DEPS = $(wildcard $(STRUG_BUILD_DIR)/*.d)
CXX_SRCS = $(shell find src -type f -name '*.cpp')
OBJS = $(patsubst src/%.cpp, $(STRUG_BUILD_DIR)/%.o, $(CXX_SRCS))

.PHONY: r rr run glade clean ass

r: ${STRUG_BUILD_DIR}/strug run

rr: glade ${STRUG_BUILD_DIR}/strug ass run

run:
	./${STRUG_BUILD_DIR}/strug

glade:
	cd deps/glade && ${MAKE}

clean:
	rm -rf build

ass:
	mkdir -p ${STRUG_BUILD_DIR}/assets
	cp -R ${DEPS_PATH}/glade/build/glade/assets/* ${STRUG_BUILD_DIR}/assets
	cp -R assets/* ${STRUG_BUILD_DIR}/assets

###################### Linkage

${STRUG_BUILD_DIR}/strug: ${DEPS_PATH}/glade/build/glade/libglade.a
${STRUG_BUILD_DIR}/strug: build/sha1/sha1.o
${STRUG_BUILD_DIR}/strug: ${OBJS}
	clang++ -O0 -g -L${DEPS_PATH}/libccd/src -L${DEPS_PATH}/zlib -L${DEPS_PATH}/assimp/lib -L${DEPS_PATH}/glew/lib/ -L${DEPS_PATH}/glade/build/glade -L${DEPS_PATH}/freetype/objs -L${DEPS_PATH}/glfw/build/src -lz -lassimp -lglfw3 -lGLEW -lfreetype -lglade -lccd -framework OpenGL -framework AppKit -framework IOKit build/sha1/sha1.o ${OBJS} -o $@

###################### Compilation

CXXFLAGS = -std=c++20 -O0 -g -DGLADE_MACOS
COMMON_INCLUDES = -Iinclude -I${DEPS_PATH}/glade/include

-include $(DEPS)

build/sha1/sha1.o: ${DEPS_PATH}/sha1/sha1.c
	mkdir -p $(@D)
	clang -O0 -g -c ${DEPS_PATH}/sha1/sha1.c -o $@

${STRUG_BUILD_DIR}/main.o: src/main.mm
	mkdir -p $(@D)
	clang -O0 -g -x objective-c -Ideps/glade/include -DGLADE_MACOS -DGL_SILENCE_DEPRECATION -c src/main.mm -o $@

${STRUG_BUILD_DIR}/main-glfw.o: src/main-glfw.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} -DGL_SILENCE_DEPRECATION ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -I${DEPS_PATH}/glfw/include -MMD -MF ${STRUG_BUILD_DIR}/main-glfw.d -c src/main-glfw.cpp -o $@

${STRUG_BUILD_DIR}/ResourceManager.o: src/ResourceManager.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -MMD -MF ${STRUG_BUILD_DIR}/ResourceManager.d -c src/ResourceManager.cpp -o $@

${STRUG_BUILD_DIR}/blocks/Sphere.o: src/blocks/Sphere.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/PerlinNoise/include -MMD -MF ${STRUG_BUILD_DIR}/Sphere.d -c src/blocks/Sphere.cpp -o $@

${STRUG_BUILD_DIR}/blocks/Frank.o: src/blocks/Frank.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/PerlinNoise/include -MMD -MF ${STRUG_BUILD_DIR}/Frank.d -c src/blocks/Frank.cpp -o $@

${STRUG_BUILD_DIR}/blocks/Triangle.o: src/blocks/Triangle.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -MMD -MF ${STRUG_BUILD_DIR}/Triangle.d -c src/blocks/Triangle.cpp -o $@

${STRUG_BUILD_DIR}/blocks/Cube.o: src/blocks/Cube.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -MMD -MF ${STRUG_BUILD_DIR}/Cube.d -c src/blocks/Cube.cpp -o $@

${STRUG_BUILD_DIR}/blocks/Isosurface.o: src/blocks/Isosurface.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/PerlinNoise/include -MMD -MF ${STRUG_BUILD_DIR}/Isosurface.d -c src/blocks/Isosurface.cpp -o $@

${STRUG_BUILD_DIR}/states/Minecraft.o: src/states/Minecraft.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -MMD -MF ${STRUG_BUILD_DIR}/Minecraft.d -c src/states/Minecraft.cpp -o $@

${STRUG_BUILD_DIR}/states/CubeTest.o: src/states/CubeTest.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -MMD -MF ${STRUG_BUILD_DIR}/CubeTest.d -c src/states/CubeTest.cpp -o $@

${STRUG_BUILD_DIR}/states/WalkingTheWorld.o: src/states/WalkingTheWorld.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -MMD -MF ${STRUG_BUILD_DIR}/WalkingTheWorld.d -c src/states/WalkingTheWorld.cpp -o $@

${STRUG_BUILD_DIR}/states/CollisionTest.o: src/states/CollisionTest.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -I${DEPS_PATH}/libccd/src -MMD -MF ${STRUG_BUILD_DIR}/CollisionTest.d -c src/states/CollisionTest.cpp -o $@

${STRUG_BUILD_DIR}/states/MarchingCubes.o: src/states/MarchingCubes.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -MMD -MF ${STRUG_BUILD_DIR}/MarchingCubes.d -c src/states/MarchingCubes.cpp -o $@

${STRUG_BUILD_DIR}/states/Chunked.o: src/states/Chunked.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -MMD -MF ${STRUG_BUILD_DIR}/Chunked.d -c src/states/Chunked.cpp -o $@

${STRUG_BUILD_DIR}/states/Craft.o: src/states/Craft.cpp
	mkdir -p $(@D)
	clang++ ${CXXFLAGS} ${COMMON_INCLUDES} -I${DEPS_PATH}/glew/include -MMD -MF ${STRUG_BUILD_DIR}/Craft.d -c src/states/Craft.cpp -o $@

