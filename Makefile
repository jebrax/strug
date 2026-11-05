ifndef DEPS_PATH
DEPS_PATH=deps
endif

ifndef STRUG_BUILD_DIR
STRUG_BUILD_DIR=build/strug
endif

# make RELEASE=1 builds optimized binaries. Run "make clean" when switching, objects are not
# tracked per configuration. NDEBUG stays off: main-glfw.cpp has side effects inside assert().
ifdef RELEASE
OPT_FLAGS = -O2 -g
else
OPT_FLAGS = -O0 -g
endif

UNAME_S := $(shell uname -s)

ifeq (${UNAME_S},Darwin)
CXX_STDLIB =
PLATFORM_LDFLAGS = -framework OpenGL -framework AppKit -framework IOKit
else
# Glade relies on libc++ headers, libc++ is linked statically so that users don't need it installed
CXX_STDLIB = -stdlib=libc++
PLATFORM_LDFLAGS = -static-libstdc++ -lGL -lX11 -lpthread -ldl -lm
endif

CXX_SRCS = $(shell find src -type f -name '*.cpp')
OBJS = $(patsubst src/%.cpp, $(STRUG_BUILD_DIR)/%.o, $(CXX_SRCS))
DEPS = $(patsubst src/%.cpp, $(STRUG_BUILD_DIR)/%.o.d, $(CXX_SRCS))

.PHONY: run-client run-server glade clean ass package

all: glade ${STRUG_BUILD_DIR}/strug ass

strug: ${STRUG_BUILD_DIR}/strug ass

GLADE_CXXFLAGS = -std=c++20 ${OPT_FLAGS} -DGLADE_MACOS -Iinclude -MMD -MF $$@.d

ifeq (${UNAME_S},Darwin)
glade:
	cd ${DEPS_PATH}/glade && ${MAKE} CXX="${CXX}" 'CXXFLAGS=${GLADE_CXXFLAGS}'
else
# Glade's own libglade.a rule needs the macOS libtool, so we only let it compile and archive ourselves
GLADE_OBJS = $(patsubst src/%.cpp, build/glade/%.o, $(shell cd ${DEPS_PATH}/glade && find src -type f -name '*.cpp'))

glade:
	cd ${DEPS_PATH}/glade && ${MAKE} CXX="${CXX}" 'CXXFLAGS=${GLADE_CXXFLAGS}' ass build/lodepng/lodepng.o ${GLADE_OBJS}
	cd ${DEPS_PATH}/glade && rm -f build/glade/libglade.a && ar rcs build/glade/libglade.a build/lodepng/lodepng.o ${GLADE_OBJS}
endif

run-client:
	./${STRUG_BUILD_DIR}/strug

run-server:
	./${STRUG_BUILD_DIR}/strug --server

clean:
	rm -rf build
	cd ${DEPS_PATH}/glade && ${MAKE} clean

ass:
	rm -rf ${STRUG_BUILD_DIR}/assets
	mkdir -p ${STRUG_BUILD_DIR}/assets
	cp -R ${DEPS_PATH}/glade/build/glade/assets/* ${STRUG_BUILD_DIR}/assets
	cp -R assets/* ${STRUG_BUILD_DIR}/assets

package:
	scripts/package.sh

###################### Linkage

${STRUG_BUILD_DIR}/strug: ${DEPS_PATH}/glade/build/glade/libglade.a
${STRUG_BUILD_DIR}/strug: build/imgui/libimgui.a
${STRUG_BUILD_DIR}/strug: build/sha1/sha1.o
${STRUG_BUILD_DIR}/strug: ${OBJS}
	${CXX} ${OPT_FLAGS} build/sha1/sha1.o ${OBJS} -o $@ -Lbuild/imgui -L${DEPS_PATH}/enet/.libs -L${DEPS_PATH}/libccd/src -L${DEPS_PATH}/zlib -L${DEPS_PATH}/assimp/lib -L${DEPS_PATH}/glew/lib/ -L${DEPS_PATH}/glade/build/glade -L${DEPS_PATH}/freetype/objs -L${DEPS_PATH}/glfw/build/src -lglade -limgui -lassimp -lfreetype -lccd -lenet -lGLEW -lglfw3 -lz ${PLATFORM_LDFLAGS}

###################### Compilation

COMMON_INCLUDES = -Iinclude -I${DEPS_PATH}/glade/include
CC = clang
CFLAGS = ${OPT_FLAGS}
CXX = clang++ ${CXX_STDLIB}
CXXFLAGS = -std=c++20 ${OPT_FLAGS} -DGLADE_MACOS ${COMMON_INCLUDES} -MMD -MF $@.d

-include $(DEPS)

${STRUG_BUILD_DIR}/%.o: src/%.cpp
	mkdir -p $(@D)
	${CXX} ${CXXFLAGS} -c $< -o $@

build/sha1/sha1.o: ${DEPS_PATH}/sha1/sha1.c
	mkdir -p $(@D)
	${CC} ${CFLAGS} -c $< -o $@

IMGUI_SRCS = imgui.cpp imgui_demo.cpp imgui_draw.cpp imgui_tables.cpp imgui_widgets.cpp backends/imgui_impl_glfw.cpp backends/imgui_impl_opengl3.cpp
IMGUI_OBJS = $(patsubst %.cpp, build/imgui/%.o, ${IMGUI_SRCS})

build/imgui/libimgui.a: ${IMGUI_OBJS}
	rm -f $@
	ar rcs $@ ${IMGUI_OBJS}

build/imgui/%.o: ${DEPS_PATH}/imgui/%.cpp
	mkdir -p $(@D)
	${CXX} -std=c++20 ${OPT_FLAGS} -I${DEPS_PATH}/imgui -I${DEPS_PATH}/glfw/include -c $< -o $@

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

.NOTPARALLEL:
