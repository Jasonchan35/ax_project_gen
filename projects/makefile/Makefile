#!! Unix Makefile
#!! Works for pmake (FreeBSD), GNUmake (GNU Linux)
#!! 

OUTPUT_DIR = ../../bin
OUTPUT_TARGET = $(OUTPUT_DIR)/ax_gen

pwd = $$(pwd)

.PHONY: build clean $(OUTPUT_TARGET)


CC = g++
CCFLAGS = -std=gnu++11 -Wall -Werror 
# CCFLAGS += -g  # Debug

$(OUTPUT_TARGET):
	@mkdir -p $(OUTPUT_DIR)
	$(CC) "$(pwd)/../../src/_single_file_build_.cpp" -o "$@" $(CCFLAGS)

all: build

build: $(OUTPUT_TARGET)

install: build
	cp -f $(OUTPUT_TARGET) /usr/local/bin/
	
clean:
	rm $(OUTPUT_TARGET)
