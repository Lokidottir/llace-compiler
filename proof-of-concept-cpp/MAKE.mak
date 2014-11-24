#compiler makefile
DEFAULT_CC=g++
CC_FLAGS=-static-libgcc -static-libstdc++ -Wall -std=c++11 -g
LD=-Ipcre -Lpcre -lpcre -lpcrecpp
GNU_CONFIGURE=yes

all: llace-ebnf

test:
	$(DEFAULT_CC) $(CC_FLAGS) -O3 src/testgrnd.cpp -o test $(LD)

llace-ebnf:
	@echo "compiling LLace EBNF evaluator..."
	$(DEFAULT_CC) $(CC_FLAGS) -O3 src/main.cpp -o llace-ebnf $(LD)

get_and_make_pcre: clean_pcre get_pcre make_pcre

make_pcre: setup_pcre
	cd pcre
	@echo "you may need to run'./pcre-8.35/configure && make' manually"
	./pcre-8.35/configure && make
	cd ../

setup_pcre:
	mkdir -p pcre

get_pcre: setup_pcre
	wget ftp://ftp.csx.cam.ac.uk/pub/software/programming/pcre/pcre-8.35.tar.gz
	tar -zxvf pcre-8.35.tar.gz
	rm pcre-8.35.tar.gz
	mv pcre-8.35 pcre

clean_pcre:
	-rm -rf pcre*

clean:
	-rm -rf llace
	-rm -rf test
