all: proj1

proj1: main.o des.o
	g++ -std=c++11 main.o des.o -o proj1

main.o: main.cpp
	g++ -c -std=c++11 main.cpp des.cpp

des.o: des.cpp
	g++ -c -std=c++11 des.cpp
