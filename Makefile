run: all
	./build/array_list

all: build/
	cd build && cmake .. -G Ninja && cmake --build .

build/:
	mkdir build
