
# Why not CMake
CMake is powerful and popular, it supports almost all platform you need and works pretty well in open source community, it's a solution to replace automake to help people manage project easier, but at the end I found that it's not as easy as what I expected, I had to spend quite some time to spot the syntax or logical error.

And here are some examples I had problem with CMake
## Just specify files for project
of cause you can specify every single cpp file one by one
but for daily work, I probably just want to put all .cpp file in a folder and compile
so in cmake I have to this:
```
FILE(GLOB MyCSources *.cpp)
ADD_EXECUTABLE(MyExecutable ${MyCSources})
```
or 
```
FILE(GLOB_RECURSE  MyCSources *.cpp)
```
if you want files in all sub-directories as well

but what if I don't want some files ? I'm not that good in CMake script, I believe there is a way to do so, but it turned out became a complicate script logic. 

And I try to search from google and I got something like this

```
set (EXCLUDE_DIR "/hide/")
file (GLOB_RECURSE SOURCE_FILES "*.cpp" "*.c")
foreach (TMP_PATH ${SOURCE_FILES})
    string (FIND ${TMP_PATH} ${EXCLUDE_DIR} EXCLUDE_DIR_FOUND)
    if (NOT ${EXCLUDE_DIR_FOUND} EQUAL -1)
        list (REMOVE_ITEM ${SOURCE_FILES} ${TMP_PATH})
    endif ()
endforeach(TMP_PATH)
```
I already get lost, even I think I can understand how it works if I take some time to read, but why I need to take so much time for just adding file to my project, I probably not started my actual coding yet.

And what I really want is:
```
"files":{"./src/**/*.cpp"}
```
for all cpp file under `src` including sub-directories, this `**` glob is quite common now, you can find it from python, visual studio code ...etc.

then if I have some cpp files excluded from the result above, I only have to do this:
```
"files":{"./src/**/*.cpp"},
"exclude_files":{
    "./src/bad_code/*.cpp",
    "./src/something_else/*.*"
}
```
simple and clean, it should be enough to describe what file I want, so why I have to do programming in my project file ?

## Precompiled Header
Precompiled Header (PCH) is a basic feature supported almost all compiler, which really can improve compile time.

In Xcode, you only have to set PCH filename in project, then just add include "pch.h" at the top for all cpp files.

In Visual Studio, an extra step is needed, usually just create pch.cpp which only include "pch.h", and set compile option from `use PCH` to `create PCH`.

In Makefile, it's a bit complicate, if you want the correct setup for file dependencies, but I'm not going to explain here.

Ok then what about CMake ?

I try to read Manual from CMake, but seems it doesn't support by default (maybe I'm wrong, if you know please tell me), 

and here some example CMake script to adding PCH compile option for Visual C++ (VC)
```
MACRO(ADD_MSVC_PRECOMPILED_HEADER PrecompiledHeader PrecompiledSource SourcesVar)
  IF(MSVC)
    GET_FILENAME_COMPONENT(PrecompiledBasename ${PrecompiledHeader} NAME_WE)
    SET(PrecompiledBinary "${CMAKE_CURRENT_BINARY_DIR}/${PrecompiledBasename}.pch")
    SET(Sources ${${SourcesVar}})

    SET_SOURCE_FILES_PROPERTIES(${PrecompiledSource}
                                PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_OUTPUTS "${PrecompiledBinary}")
    SET_SOURCE_FILES_PROPERTIES(${Sources}
                                PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\" /FI\"${PrecompiledHeader}\" /Fp\"${PrecompiledBinary}\""
                                           OBJECT_DEPENDS "${PrecompiledBinary}")  
    # Add precompiled header to SourcesVar
    LIST(APPEND ${SourcesVar} ${PrecompiledSource})
  ENDIF(MSVC)
ENDMACRO(ADD_MSVC_PRECOMPILED_HEADER)
```
It's just for VC, what about gcc or clang ? you need more lines of CMake script

And I still have to add #include "pch.h" in every single cpp file. Ok no problem, VC can do force-inline header by compile option.

If you don't why to do such CMake script yourself, you can try to find some 3rd party module, but the question is which one is good ? what compile or platform they support ? do they support force-include header for all compilers ?

And at the end of the day, I just want to tell compiler this is my precompiled header, so in ax_gen here is the only line you need
```
"pch_header":"my_pch.h"
```
and it will 
- add compile option depends on compiler
- force-include for all cpp files
- generate pch.cpp to create PCH object file for Visual C++

My point is why we need some many option or step to setup PCH, I just need a header file which will compile and consumed by other cpp files to save compile time, what else you need to tell other than the pch file name ?

## Conclusion
I think at the end it's about how to get the job done, CMake is powerful and flexible, but when writing CMakefile.txt just like coding, it sounds a bit over-engineering to me.

In fact if you see how people using Visual C++ or Xcode, they don't have any problem about how to manage files or compile option in project, and it's all the information compiler need.

For me I'm happy with those IDE, but the only problem I have is cross-platform, when I try to maintain a project which have to compile on Windows, Mac, Linux, FreeBSD, iOS, Android ...etc, and they all using different IDE, it became a nightmare to me

Therefore I make this project generator, just read a json file for all information is needed then output target IDE project for my actual work.