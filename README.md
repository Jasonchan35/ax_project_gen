# ax_project_gen
C++ Project Generator

### Feature Highlights:
* Lightweight - only require C++11, libuuid and good to go! *( No python, java or bunch of libraries / packages required )*

* Able to run on multiple platforms *( Windows / MacOSX / Linux / FreeBSD )*

* Auto inherit configuration from depended project, for instance executable project depends on library <br> will inherit all settings by default *( e.g. include_dir, cpp_defines, output_library ...etc )*

* Support wildcard, glob with sub-directories *( example: ```src/**/*.cpp```,  means all c++ files within `src` folder including all sub-directories )*

* Precompiled header - also apply force-include to all c++ files<br>
`(*In vs2015 precompiledHeader.cpp will be auto generated)`

* Unite Build - build multiple small C/C++ file at once to improve the compile time

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
| -gen		| generate target projects     |
| -build	| build projects               |
| -ide		| open IDE                     |
| -run		| run startup project          |
| -verbose  | more detail in console / log |

|**Generators:**|||
|------------------|-----------------------------------------|---|
| gen=vs2015       | Visual Studio 2015                      ||
| gen=vs2015_linux | Visual Studio 2015 Linux Remote Compile ||
| gen=xcode        | Xcode                                   ||
| gen=makefile     | GNU/BSD makefile format | `*support file path with space` <br>`(Which handwritten Makefile might have problem`<br>`during 2nd degree variable evaluation)` |

|**OS options:**||
|------------|------------|
| os=windows | Windows |
| os=macosx  | MacOSX     |
| os=ios     | iOS Device |
| os=linux   | Linux      |
| os=freebsd | FreeBSD    |

|**CPU Options:**||
|------------|------------|
| cpu=x86    | Intel / AMD 32-bit CPU  |
| cpu=x86_64 | Intel / AMD 64-bit CPU  |

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