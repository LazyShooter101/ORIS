prog := task2_nonCRT_speedup

all: build run

build:
	gcc .\$(prog).c -o $(prog) -lgmp

run:
	./$(prog).exe