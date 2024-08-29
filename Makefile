BUILD_DIR = build
BIN = ${BUILD_DIR}/array_list

TEST = ${BUILD_DIR}/tests/test

export VCPKG_ROOT

DEBUG_FLAGS = -DCMAKE_BUILD_TYPE=Debug
RELEASE_FLAGS = -DCMAKE_BUILD_TYPE=Release

run: debug
	./${BIN}

debug: ${BUILD_DIR}/
	cd build && \
	cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ${DEBUG_FLAGS} && \
	cmake --build .

release: ${BUILD_DIR}/
	cd build && \
	cmake .. -G Ninja -DCMAKE_TOOLCHAIN_FILE="${VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake" ${RELEASE_FLAGS} && \
	cmake --build .

test: debug
	./${TEST}

build/:
	mkdir build