ifndef DEPS_PATH
DEPS_PATH=deps
endif

ifndef STRUG_BUILD_DIR
STRUG_BUILD_DIR=build/strug
endif

CXX_SRCS = $(shell find src -type f -name '*.cpp')
OBJS = $(patsubst src/%.cpp, $(STRUG_BUILD_DIR)/%.o, $(CXX_SRCS))
DEPS = $(patsubst src/%.cpp, $(STRUG_BUILD_DIR)/%.o.d, $(CXX_SRCS))

.PHONY: r rr run glade clean ass

all: glade ${STRUG_BUILD_DIR}/strug ass run

strug: ${STRUG_BUILD_DIR}/strug ass

glade:
	cd ${DEPS_PATH}/glade && ${MAKE}

run:
	./${STRUG_BUILD_DIR}/strug

clean:
	rm -rf build

ass:
	rm -rf ${STRUG_BUILD_DIR}/assets
	mkdir -p ${STRUG_BUILD_DIR}/assets
	cp -R ${DEPS_PATH}/glade/build/glade/assets/* ${STRUG_BUILD_DIR}/assets
	cp -R assets/* ${STRUG_BUILD_DIR}/assets

###################### Linkage

${STRUG_BUILD_DIR}/strug: ${DEPS_PATH}/glade/build/glade/libglade.a
${STRUG_BUILD_DIR}/strug: ${DEPS_PATH}/imgui/build/imgui/libimgui.a
${STRUG_BUILD_DIR}/strug: build/sha1/sha1.o
${STRUG_BUILD_DIR}/strug: ${OBJS}
	clang++ -O0 -g -L${DEPS_PATH}/imgui/build/imgui -L${DEPS_PATH}/libccd/src -L${DEPS_PATH}/zlib -L${DEPS_PATH}/assimp/lib -L${DEPS_PATH}/glew/lib/ -L${DEPS_PATH}/glade/build/glade -L${DEPS_PATH}/freetype/objs -L${DEPS_PATH}/glfw/build/src -lz -lassimp -lglfw3 -lGLEW -lfreetype -lglade -lccd -limgui -framework OpenGL -framework AppKit -framework IOKit build/sha1/sha1.o ${OBJS} -o $@

###################### Compilation

COMMON_INCLUDES = -Iinclude -I${DEPS_PATH}/glade/include
CC = clang
CFLAGS = -O0 -g
CXX = clang++
CXXFLAGS = -std=c++20 -O0 -g -DGLADE_MACOS ${COMMON_INCLUDES} -MMD -MF $@.d

-include $(DEPS)

${STRUG_BUILD_DIR}/%.o: src/%.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -c $< -o $@

build/sha1/sha1.o: ${DEPS_PATH}/sha1/sha1.c
	mkdir -p $(@D)
	${CC} ${CFLAGS} -c $< -o $@

${STRUG_BUILD_DIR}/main-glfw.o: src/main-glfw.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -DGL_SILENCE_DEPRECATION -I${DEPS_PATH}/imgui -I${DEPS_PATH}/imgui/backends -I${DEPS_PATH}/glew/include -I${DEPS_PATH}/glfw/include -c $< -o $@

${STRUG_BUILD_DIR}/states/world/WalkingTheWorld.o: src/states/world/WalkingTheWorld.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${DEPS_PATH}/imgui -c $< -o $@

${STRUG_BUILD_DIR}/states/world/TerrainDemo.o: src/states/world/TerrainDemo.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${DEPS_PATH}/imgui -c $< -o $@

${STRUG_BUILD_DIR}/states/CollisionTest.o: src/states/CollisionTest.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -I${DEPS_PATH}/glew/include -I${DEPS_PATH}/libccd/src -c $< -o $@

