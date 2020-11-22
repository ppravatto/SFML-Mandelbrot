COMP=g++
FLAGS=-lsfml-graphics -lsfml-window -lsfml-system

all: main.cpp
	$(COMP) main.cpp -c
	$(COMP) main.o -o main.exe $(FLAGS)

parallel: main.cpp
	$(COMP) main.cpp -c -fopenmp
	$(COMP) main.o -o main.exe -fopenmp $(FLAGS)

clear:
	rm -rf main.o
	rm -rf main.exe
