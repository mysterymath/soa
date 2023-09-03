all: main

main: main.cc
	clang++ -O2 -fno-inline -std=c++20 -I. -o main main.cc
