prog := task2_nonCRT

all: build run

build:
	gcc .\$(prog).c -o $(prog) -lgmp

run:
	./$(prog).exe