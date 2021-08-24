@echo off
set VISUAL_STUDIO_DIR=c:\Programs\VS2019\VC\Auxiliary\Build\
cls
del testdll_x32.dll
del testdll_x64.dll

call "%VISUAL_STUDIO_DIR%\vcvarsall" x86
CD /D "%~dp0"
"cl.exe" /LD /O1 /Ob2 /Os /GS- /GL /Oy- /Zc:inline /MT main.cpp ..\msvcrt.x32.lib user32.lib kernel32.lib imagehlp.lib /link /SAFESEH:NO /ENTRY:DllMain /NODEFAULTLIB /DRIVER /OUT:testdll_x32.dll

call "%VISUAL_STUDIO_DIR%\vcvarsall" x64
CD /D "%~dp0"
"cl.exe" /LD /O1 /Ob2 /Os /GS- /GL /Oy- /Zc:inline /MT main.cpp ..\msvcrt.x64.lib user32.lib kernel32.lib imagehlp.lib /link /SAFESEH:NO /ENTRY:DllMain /NODEFAULTLIB /DRIVER /OUT:testdll_x64.dll


del *.obj *.exp *.ipdb *.iobj main.lib
pause
