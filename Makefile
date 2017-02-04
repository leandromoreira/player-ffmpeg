exec1:
	gcc -g -Wall -o build/player1 -lavformat -lavcodec -lswscale -lz part1.c && ./build/player1 bunny_1080p.mp4
