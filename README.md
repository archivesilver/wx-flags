# Using Codelite with static wxWidgets 

## Description
- This project aims to create a functioning CodeLite environment that integrates with **static** wxWidgets libraries. 
- Target OS is **Windows** generally but it shouldn't be hard to use it with very little change for other systems. 
- C++ oriented. 
- Designed for wxWidgets v3.2.5, so deprecated libraries are supposed to be added manually. 
- This isn't a standalone project. You will still need to have Codelite and wxWidgets. Also, this project doesn't really care to properly fix the mess, it is simply a workaround that tries to respect the official release versions as much as possible. 

## Problem Statement
There is a supposed helper tool called wx-config. There is a Windows port of that tool, originally called wx-config-win. It is a part of CodeLite. A quote from wx-config-msys2's readme: 
> The puropse is tool is to allow easy integration between IDEs and wxWidgets.. 

It does as much as it says. Following the same spirit: 

The puropse is tool is to allow easy integration between wx-config and everything else. 

Joking aside, wx-config fails to work with the static version of wxWidgets libraries on several levels: 
- Statically-built wxWidgets libraries will not produce a *build.cfg* file. wx-config depends on that file.
- Even when an artificial build.cfg file is introduced, wx-config fails to set the correct flags/options. 

In addition, Codelite sample GUI projects depend on wx-config and there seems to be no resource among either Codelite or wxWidgets documentation that has a list of available flags. Funnily, you will see that pretty much both documentations will rely on wx-config when it comes to these flags. Apparently there are more flags that can be used with wx-config, but they also don't work. 

## Approach 
It is possible to have a thorough read of wx-config-win.cpp and come up with a list of available flags. If that's your alley, have fun doing that. Also there is a `config` folder created in `lib/wx` inside build directory so it could be hintful, but I didn't see any *--rcflags* option in those and its date is even earlier. So I am guessing it's not windows-oriented or more useful.
Here we will focus on manipulating the output of wx-config, so, ironically, we will also depend on wx-config. Here are the steps to our method: 
1. Produce the infamous *build.cfg* file and place it where it could be. 
2. Have wx-config produce some flags. 
3. Rectify the flags. 

# Preparation
Take a look at [installing and building wxWidgets](Installing_wxWidgets.md) if you're interested in seeing what kind of setup this tool is designed to work on, or creating an equivalent environment. 
Codelite installation is not documented, because it doesn't seem related to how this tool will work. WXWIN and WXCFG environment variables are defined. 

# Installation
The current version is a single .cpp file. Compile the file and move the created executable next to `wx-config.exe`, which is typically located in `C:/Program Files/CodeLite/bin`. 

# Usage
Add wx-flags.exe's directory into PATH. Codelite's Environment Variables can be used: 
- In Codelite Menu, select Settings → Environment Variables...
- Add this line: `PATH=C:\Program Files\CodeLite\bin;$PATH` (PATH should be equal to CodeLiteDir environment variable plus `\bin`.)
Right click on the project folder, located on the left sidebar and select *Properties*.
To use the tool in an existing project, make the following changes:
- In `Compiler` section, next to `C++ Compiler Options`, replace `wx-config` with `wx-flags`. 
- In `Linker` section, next to `Linker Options`, replace `wx-config` with `wx-flags`. 
- In `Resources` section, next to `Resource Compiler Options`, replace `wx-config` with `wx-flags`. 
- If you wish, you can do these replacements in the `Global Settings` instead. 
- After running the tool (or the project configured to run it) once, take a look at WXWIN/lib/WXCFG/build.cfg file. Edit predefined values if necessary:
	- WXVER_RELEASE=5
	- BUILD=debug
	- MONOLITHIC=0
	- SHARED=0
	- WXUNIV=0
	- VENDOR=custom
	- OFFICIAL_BUILD=0
	- DEBUG_FLAG=0
	- DEBUG_INFO=0
	- RUNTIME_LIBS=static
	- USE_RTTI=1
	- USE_QA=1
	- COMPILER=clang
	- CC=cc
	- CXX=c++

## Notes
- Since this tool will deliver all the flags to wx-config, you don't have to worry about it. 
- Use `--wxflagsDYNAMIC` for projects that are supposed to have dynamic linking. 
- This tool assumes you have built wxWidgets inside the source code folder whose name includes "wxWidgets", and the build folder's name doesn't include "wxWidgets". If your setup doesn't fit to that, you can adjust folders' names, or edit `fixInclude` function before compiling the tool. 
- Unlike wx-config, if you specify multiple flag options wx-flags will misbehave. Don't use it with multiple flags. 
- The tool is only designed to fix the flags. If you wish to use other native wx-config options, do so by adding `--wxflagsDYNAMIC` flag. 
- Some libraries are added based on values defined in `setup.h` file located in `\lib\wx\include\msw-unicode-static-3.2\wx` (in wxWidgets build directory). 
- Some libraries are added based on what wx-config seems to be designed to add. The tool adds all of those by default. If you wish to fine-tune these additions, use `--wxflagsCUSTOM` and manually add the libraries you want. 
- The tool is heavily untested except for target setup. 


## Persistent solution: Template
To have this problem permanently solved, you can save a project as template. 
- Start an empty project. 
- Apply the steps described in the sections above. 
- Right click on the project in the left sidebar and select `Save as template...` 

# Troubleshooting
- If you receive errors about libraries that couldn't be found, it is probably because wxWidgets team has decided to change the library naming conventions. As a workaround, you can try renaming your libraries to what the tool is expecting. 
- If you receive errors stating that the tool is configured incorrectly, it means the original wx-config tool is not working the way it's supposed to. Make sure you can first use wx-config without this tool without receiving unusual errors. 
- If you receive errors about undefined or replaced symbols, it is probably about a missing library or the library order. I will try to update the order if I get feedback. 

# Additional Tips
These aren't really related to the tool, but while we're at it, I figured I would list the other things I have done to work more comfortably with static linking. 

## Optimizing for File Size
Project Settings → Compiler → C++ Compiler Options:  
  * Use -Os to optimize for size, check if you have another -O flag to avoid conflicts.
  * -flto

Project Settings → Linker → Linker options: 
  - -Wl,--gc-sections
  - -static-libgcc
  - -static-libstdc++
  - -static
  - -flto  
	Some of these are available as checkable options if you click on the 3-dot button.

Project Settings → Post Build Commands: 
  - Add `strip --strip-unneeded $(WorkspacePath)/build-$(WorkspaceConfiguration)/bin/$(ProjectName).exe` (No need to replace anything)
  - This is the most effective option to reduce the size. (I have got ~82%/48MB reduced size on an almost empty project.)
  - This is gonna remove symbols not required for execution, so if you're using debug configuration, you can ignore this. 

# Contribution
The project is limited to my use case: Windows, C++. If you wish to contribute, you could send a PR, however it's better if you actually work on the [original wx-config-win](https://github.com/eranif/codelite/blob/master/sdk/wxconfig/wx-config-win.cpp) and fix it instead. 

# License
This project is free and libré, no support is promised, no warranties. 
I would appreciate if you credited me where appropriate. 