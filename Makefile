all: main

clean:
	-rm main

main: main.cc
	${HOME}/llvm-mos/build/bin/mos-sim-clang++ -Os -fno-lto -fnonreentrant -std=c++20 -I. main.cc
