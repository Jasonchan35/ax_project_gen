#!! Unix Makefile
#!! Works for pmake (FreeBSD), GNUmake (GNU Linux)
#!! 

.PHONY: build clean

OUTPUT_TARGET = ../../bin/ax_gen

CC = g++
CCFLAGS = -std=c++11 -Wall -Werror -g

UNAME := $(shell uname)

ifeq ($(UNAME), Darwin)
	CC = clang++
	CCFLAGS += -lSystem
else
	CCFLAGS += -luuid
endif

$(OUTPUT_TARGET):
	@mkdir -p ./bin
	$(CC) ../../src/_single_file_build_.cpp -o "$@" $(CCFLAGS)

all: build

build: $(OUTPUT_TARGET)
	
clean:
	rm $(OUTPUT_TARGET)
