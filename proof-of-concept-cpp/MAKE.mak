#compiler makefile
CC=g++
CC_FLAGS=-static -static-libgcc -static-libstdc++ -Wall -std=c++11 -g
LD=

all: clc 

test:
	@echo "no test program at this time"

clc:
	@echo "compiling clc..."
	$(CC) $(CC_FLAGS) -O3 src/main.cpp -o clc $(LD)
	
clean:
	rm clc
