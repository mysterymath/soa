all: main.s

clean:
	-rm main.s

main.s: main.cc soa.h soa-struct.inc
	${HOME}/llvm-mos/build/bin/mos-sim-clang++ -Os -fno-lto -fnonreentrant -I. -S main.cc
