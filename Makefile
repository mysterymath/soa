all: main

main: main.cc
	clang++ -O2 -fno-inline -I. -o main main.cc
