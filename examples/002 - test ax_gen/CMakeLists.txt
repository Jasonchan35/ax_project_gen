cmake_minimum_required (VERSION 2.8.11)
project (ax_gen)

FILE(GLOB_RECURSE  MyCSources ../../src/*.cpp)
ADD_EXECUTABLE(MyExecutable ${MyCSources})
