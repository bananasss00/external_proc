@echo off
set VISUAL_STUDIO_DIR=c:\Programs\VS2019\VC\Auxiliary\Build\
call "%VISUAL_STUDIO_DIR%\vcvarsall" x86
cls
del dlltest.exe
CD /D "%~dp0"
"cl.exe" /Od /std:c++11 /MT /D_TEST dllmain.cpp user32.lib kernel32.lib imagehlp.lib /link /EHsc /OUT:dlltest.exe
del *.obj *.exp *.ipdb *.iobj dllmain.lib
dlltest.exe
pause
