#!! Unix Makefile
#!! Works for pmake (FreeBSD), GNUmake (GNU Linux)
#!! 

OUTPUT_TARGET = ../../bin/ax_gen

.PHONY: build clean $(OUTPUT_TARGET)


CC = g++
CCFLAGS = -std=c++11 -Wall -Werror -g

$(OUTPUT_TARGET):
	@mkdir -p $(dir $(OUTPUT_TARGET))
	$(CC) ../../src/_single_file_build_.cpp -o "$@" $(CCFLAGS)

all: build

build: $(OUTPUT_TARGET)
	
clean:
	rm $(OUTPUT_TARGET)
