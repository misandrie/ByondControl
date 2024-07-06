#pragma once
#include <windows.h>
#include <detours.h>
#include "byond.h"
#include "dllmain.h"
#include "pipes.h"

DWORD spoofedVolumeSerialNumber = 0;

char replacementHash[33];
char* __cdecl md5_hook(char* arg) {
    char* result = md5(arg);
    if (ShouldChangeHash()) {
        strncpy_s(result, 33, replacementHash, 33);
    }
    return result;
}

void StartCidSpoofer()
{
    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Dantom\\BYOND", NULL, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        char buffer[33];
        buffer[32] = 0;
        DWORD dwSize = sizeof(buffer);
        //DWORD regType = REG_SZ;
        if (RegQueryValueExA(hKey, "hashspoof", NULL, NULL, (LPBYTE)&buffer, &dwSize) == ERROR_SUCCESS) {
            strncpy_s(replacementHash, 33, buffer, 33);
            PIPELOG("dseeker received hash: %s\n", replacementHash);
        }
        RegCloseKey(hKey);
    }
    else {
        PIPELOG("failed to open registry\n");
        ExitProcess(-1);
    }

	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)md5, md5_hook);
	DetourTransactionCommit();
}