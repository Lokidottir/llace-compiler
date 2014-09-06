#compiler makefile
DEFAULT_CC=g++
CC_FLAGS=-static -static-libgcc -static-libstdc++ -Wall -std=c++11 -g
LD=

all: clc

test:
	@echo "no test program at this time"

clc:
	@echo "compiling clc..."
	$(DEFAULT_CC) $(CC_FLAGS) -O3 src/main.cpp -o clc $(LD)

get_and_make_pcre: clean_pcre get_pcre setup_pcre make_pcre

make_pcre:
	cd pcre_obj
	pcre_obj/../pcre-8.35/configure
	make
	cd ../

setup_pcre:
	mkdir pcre_obj
	
get_pcre:
	wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.35.tar.gz
	tar -zxvf pcre-8.35.tar.gz
	rm pcre-8.35.tar.gz

clean_pcre:
	rm -r pcre_obj
	rm -r pcre-8.35

clean:
	rm -r clc
