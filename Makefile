prog := task1_in_c

all: build run

build:
	gcc .\$(prog).c -o task1_in_c -lgmp

run:
	./$(prog).exe