default:
	gcc -Wall -o build/player1 -lavformat part1.c
exec1:
	./build/player1 bunny_1080p.mp4
