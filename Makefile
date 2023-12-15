all: main rewrite

main: main.c
	gcc main.c RS-232/rs232.c -o main -lSDL2 -lSDL2_ttf

rewrite: rewrite.c
	gcc rewrite.c RS-232/rs232.c -o rewrite -lSDL2 -lSDL2_ttf
