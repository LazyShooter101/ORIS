prog := task2_in_c

all: build run

build:
	gcc .\$(prog).c -o $(prog) -lgmp

run:
	./$(prog).exe