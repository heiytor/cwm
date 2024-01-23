all: build

build:
	gcc main.c -o main -Wall -Wextra -std=c17 -lX11
