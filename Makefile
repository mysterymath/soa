all: main

clean:
	-rm main

main: main.cc soa.h soa-struct.inc
	${HOME}/llvm-mos/build/bin/mos-sim-clang++ -Os -I. -o main main.cc
