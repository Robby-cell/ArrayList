BUILD_DIR = build

TEST = ${BUILD_DIR}/Debug/tests/run-tests

debug:
	conan install . --build=missing -sbuild_type=Debug -pr=clang   && \
	conan build . -sbuild_type=Debug -pr=clang

release:
	conan install . --build=missing -sbuild_type=Release -pr=clang && \
	conan build . -sbuild_type=Release -pr=clang

test: debug
	./${TEST}
