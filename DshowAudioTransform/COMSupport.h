#pragma once

#include <streams.h>

BOOL APIENTRY DllMain(HANDLE hModule, DWORD dwReason, LPVOID lpReserved);
STDAPI DllRegisterServer(void);
STDAPI DllUnregisterServer();