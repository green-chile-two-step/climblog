all: climblog.cpp
	g++ -std=c++14 -g -O3 -Wall -Wextra climblog.cpp -o climblog

clean:
	rm -rf climblog climblog.dSYM
