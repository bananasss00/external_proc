@echo off
set VISUAL_STUDIO_DIR=c:\Programs\VS2019\VC\Auxiliary\Build\
cls
del get_proc_x32.exe
del get_proc_x64.exe

call "%VISUAL_STUDIO_DIR%\vcvarsall" x86
CD /D "%~dp0"
"cl.exe" /O1 /Ob2 /Os /GS- /GL /Oy- /Zc:inline /MT main.cpp ..\msvcrt.x32.lib user32.lib kernel32.lib imagehlp.lib /link /ENTRY:main /SAFESEH:NO /NODEFAULTLIB /FILEALIGN:16 /ALIGN:16 /DRIVER /OUT:get_proc_x32.exe

call "%VISUAL_STUDIO_DIR%\vcvarsall" x64
CD /D "%~dp0"
"cl.exe" /O1 /Ob2 /Os /GS- /GL /Oy- /Zc:inline /MT main.cpp ..\msvcrt.x64.lib user32.lib kernel32.lib imagehlp.lib /link /ENTRY:main /SAFESEH:NO /NODEFAULTLIB /FILEALIGN:16 /ALIGN:16 /DRIVER /OUT:get_proc_x64.exe


del *.obj *.exp *.ipdb *.iobj main.lib
pause
