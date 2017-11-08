MU := $(shell pkg-config --libs xmu)

all : example1 gist

example1 : example1.cpp
	g++ $< -o $@ -lX11

gist : ./gistfile1.c
	gcc $< -o $@ $(MU)

