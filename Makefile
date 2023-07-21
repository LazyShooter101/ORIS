prog := task2_shamir

all: build run

build:
	gcc .\$(prog).c -o $(prog) -lgmp -O2

run:
	./$(prog).exe