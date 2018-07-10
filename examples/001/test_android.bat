@REM #change current directory to this file
%~d0
cd %~dp0

..\..\bin\ax_gen.exe ws=Hello.axworkspace gen=android -gen

set NDK_PATH=%LOCALAPPDATA%\Android\Sdk\ndk-bundle

@REM %NDK_PATH%\ndk-build.cmd -C _build/Hello-android -j
%NDK_PATH%\ndk-build.cmd -C _build/Hello-android
