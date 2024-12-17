all: eatadakimasu

eatadakimasu: eatadakimasu_v1.0.c
	gcc -o eat eatadakimasu_v1.0.c -lncurses
