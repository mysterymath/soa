all: main.s

clean:
	-rm main.s

main.s: main.cc soa.h soa-impl.inc
	${HOME}/llvm-mos/build/bin/mos-sim-clang++ -Os -fno-lto -fnonreentrant -std=c++20 -I. -S main.cc
