all: main.s

clean:
	-rm main main.s

main.s: main.cc
	${HOME}/llvm-mos/build/bin/mos-sim-clang++ -Os -fno-lto -fnonreentrant -std=c++20 -I. -S main.cc
