#pragma once
#include <windows.h>
#include <TlHelp32.h>
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <tuple>
#include <filesystem>
#include <algorithm>
#include <imagehlp.h>
#include <psapi.h>
#include <memory>

using namespace std;

// universal pointer for x32/x64 target apps
using ptr_t = uint64_t;
using pid_t = int32_t;
using whandle_t = uint64_t;

#if defined(_M_X64) || defined(__amd64__)

#else

#endif

#ifndef _TEST
    #include <pybind11\pybind11.h>
    #include <pybind11\stl.h>
    namespace py = pybind11;
#endif