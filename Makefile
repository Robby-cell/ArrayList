BUILD_DIR = build

TEST = ${BUILD_DIR}/Debug/tests/run-tests

debug:
	conan install . --build=missing -sbuild_type=Debug -pr=default   && \
	conan build . -sbuild_type=Debug -pr=default

release:
	conan install . --build=missing -sbuild_type=Release -pr=default && \
	conan build . -sbuild_type=Release -pr=default

test: debug
	./${TEST}
