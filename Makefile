all: main

main: main.c
	gcc main.c -o main -lSDL2 -lSDL2_ttf
