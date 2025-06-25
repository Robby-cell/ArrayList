BUILD_DIR = build
BIN = ${BUILD_DIR}/Debug/array_list

TEST = ${BUILD_DIR}/Debug/tests/run-tests

run: debug
	./${BIN}

debug:
	conan install . --build=missing -sbuild_type=Debug -pr=clang   && \
	conan build . -sbuild_type=Debug -pr=clang

release:
	conan install . --build=missing -sbuild_type=Release -pr=clang && \
	conan build . -sbuild_type=Release -pr=clang

test: debug
	./${TEST}
