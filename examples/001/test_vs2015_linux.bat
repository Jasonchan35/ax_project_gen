@REM #change current directory to this file
%~d0
cd %~dp0

..\..\bin\ax_gen.exe ws=Hello.axworkspace gen=vs2015_linux -gen 
