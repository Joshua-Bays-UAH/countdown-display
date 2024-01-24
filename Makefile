all: main

main: main.c
	gcc main.c RS-232/rs232.c -o main -lSDL2 -lSDL2_ttf -pthread

cmd-manual: cmd-manual.tex
	pdflatex cmd-manual.tex
	pdflatex cmd-manual.tex
	@clean-latex
