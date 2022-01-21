#pragma once
#pragma comment(lib, "Shlwapi.lib")

#define _WIN32_WINNT 0x0601

#include <Windows.h>
#include <shlwapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <Python.h>

#define DS_STREAM_RENAME L":wtfbbq"
#define DS_DEBUG_LOG(msg) wprintf(L"\n[LOG] - %s\n", msg)
