
SRCPATH = *.cpp

Main: main.cpp
	g++ -lncurses -o WolfsAndRabbits -std=c++17 -pthread ${SRCPATH}