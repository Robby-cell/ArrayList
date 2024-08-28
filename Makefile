run: all
	./build/dump.exe

all: build/
	cd build && cmake .. -G Ninja && cmake --build .

build/:
	mkdir build
