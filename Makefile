all:
	gcc -W -Wall -Werror -std=c23 -O2 -fanalyzer Projet.c -o bin2png -lpng -lm

clean:
	rm -f bin2png *.png
