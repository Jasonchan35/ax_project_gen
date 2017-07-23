@REM #change current directory to this file
%~d0
cd %~dp0

..\..\bin\ax_gen.exe ws=cuda_support.axworkspace gen=vs2015 -gen
