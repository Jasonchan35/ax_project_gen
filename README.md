# ax_project_gen
C++ Project Generator

```
ax_pjgen.exe gen=<Geneartor Name> ws=<Workspace File> -gen
```
Example:
```
ax_pjgen.exe gen=vs2015 ws=Hello.workspace -gen
```

## gen=vs2015
Visual Studio 2015 Generator

supported os:
* windows

## gen=xcode
Xcode Project Generator

supported os:
* macosx
* ios
    
## gen=makefile
Makefile
* support GNU make (make on Linux, gmake on FreeBSD)
* support BSD make (make on FreeBSD / Solaris, pmake on Linux)
