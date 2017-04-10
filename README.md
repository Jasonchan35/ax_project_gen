# ax_project_gen
C++ Project Generator

### Feature Highlights:
* Lightweight - only require C++11 and good to go! *( No lua, python, java or any of libraries / packages dependencies )*

* Run on multiple platforms *( Windows / MacOSX / Linux / FreeBSD )*

* Auto inherit configuration from depended project, for instance executable project depends on library <br>
will inherit all settings by default *( e.g. include_dir, cpp_defines, output_library ...etc )*

* Support wildcard, glob with sub-directories *( example: ```src/**/*.cpp```,
means all c++ files within `src` folder including all sub-directories )*

* Support file path with space and Unicode for non-Latin characters

* Precompiled header - auto apply force-include to c++ files<br>
`(*In vs2015 precompiledHeader.cpp will be auto generated)`

* Unite Build - build multiple small C/C++ file at once to improve the compile time

* Generate proejcts / Makefile with multi-thread build support

* Support Project Group, Virtual Folder in Visual Studio solution or Xcode workspace

**Command Line:**
```
ax_gen.exe ws=<Workspace File> [gen=<Geneartor>] [os=<target OS>] [cpu=<target CPU>] [-gen] [-build] [-ide] [-run] [-verbose] 
```
- `arguments can be in random order`

**Example:**
```
ax_gen.exe ws=examples/001/Hello.axworkspace -gen
```

|**Actions:**||
|-----------|------------------------------|
| -gen	    | generate target projects     |
| -build    | build projects               |
| -ide	    | open IDE                     |
| -run	    | run startup project          |
| -verbose  | more detail in console / log |

|**OS options:**||
|------------|------------|
| os=windows | Windows    |
| os=macosx  | MacOSX     |
| os=ios     | iOS Device |
| os=linux   | Linux      |
| os=freebsd | FreeBSD    |

|**CPU Options:**||
|------------|------------|
| cpu=x86    | Intel / AMD 32-bit CPU  |
| cpu=x86_64 | Intel / AMD 64-bit CPU  |

|**Generators:**|||
|------------------|-----------------------------------------|---|
| gen=vs2015       | Visual Studio 2015<br>*(default on Windows)*  ||
| gen=vs2015_linux | Visual Studio 2015 Linux Remote Compile ||
| gen=vs2017       | Visual Studio 2017<br>*(default on Windows)*  ||
| gen=vs2017_linux | Visual Studio 2017 Linux Remote Compile ||
| gen=xcode        | Xcode<br>*(default on MacOSX)* ||
| gen=makefile     | GNU/BSD makefile format<br>*(default on Linux/FreeBSD)* | `*support file path with space` <br>`(Which handwritten Makefile might have problem`<br>`during 2nd degree variable evaluation)` |

|**Compiler options:**||
|----------------|-----------------|
| compiler=vc    | MS Visual C++<br>*(default in gen=vs2015)*   |
| compiler=gcc   | GNU C/C++<br>*(default in gen=makefile)*       |
| compiler=clang | clang from LLVM<br>*(default in gen=xcode)* |


<br>
<br>

--------

### Example output: ( *Visual Studio / Xcode / Makefile* )
![Visual Studio Solution](doc/ScreenShots/2017-04-03.png)

### Example Input: *Hello.axproj*:
```javascript
{
	"group": "MyGroup/MyProgram",
	"type": "cpp_exe",
	"dependencies": ["MyLib"],
	"pch_header": "src/precompiledHeader.h",

	//"unite_build": false,
	//"unite_mega_byte_per_file": 1,
	"files" : [
		"src/*.cpp",
		"*.axproj"
	],
	"exclude_files": [
	],		

	"config": {
		"cpp_defines": [],
		"cpp_flags": [],
		"include_dirs": ["src"],
		"link_dirs": [],
		"link_files": [],
		"link_flags": [],
		"compiler==gcc": {
			"link_flags":["-lm"]			
		},
		"compiler==clang": {
			"link_flags":["-lm"]			
		}
	}
}

```

# If you're interested to
* [Another generator again ? Why Not XYZ ... please click here](doc/Why_Not_XYZ.md)