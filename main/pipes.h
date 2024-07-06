#pragma once
#include <Windows.h>
#include <string>

void PIPELOG(const char* fmt, ...);
void PipeExecute(std::string msg);
DWORD WINAPI PipeHandler();