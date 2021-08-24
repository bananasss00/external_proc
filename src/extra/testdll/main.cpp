#define _NO_CRT_STDIO_INLINE
#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <algorithm>
#include <imagehlp.h>


BOOL WINAPI DllMain(HINSTANCE hinstDll, DWORD fdwReason, LPVOID lpvReserved)
{
	if (fdwReason == DLL_PROCESS_ATTACH) {
		DisableThreadLibraryCalls(hinstDll);
#if defined(_M_X64) || defined(__amd64__)
        MessageBoxA(0, "dll injected!", "X64DLL", MB_OK);
#else
        MessageBoxA(0, "dll injected!", "X32DLL", MB_OK);
#endif
	}
	return TRUE;
}