#pragma once

#include "pipes.h"
#include <string>
#include <cstdarg>
#include <Windows.h>

void PIPELOG(const char* fmt, ...)
{
	char fmtted[1000];
	va_list vl;
	va_start(vl, fmt);
	vsnprintf(fmtted, sizeof(fmtted), fmt, vl);
	va_end(vl);
	printf(fmtted);

	DWORD read = 0;
	CallNamedPipeA("\\\\.\\pipe\\byondcontrol13log", fmtted, sizeof(fmtted), nullptr, 0, &read, NMPWAIT_WAIT_FOREVER);
}

void PipeExecute(std::string msg)
{
	size_t space = msg.find(' ');
	std::string cmd;
	std::string arg;
	if (space == std::string::npos) {
		cmd = msg.substr(0, std::string::npos);
		arg = std::string("");
	}
	else {
		cmd = msg.substr(0, space);
		arg = msg.substr(space, std::string::npos); // to the end
	}

	if (cmd == "id") {
		arg.c_str();
	}
}

DWORD WINAPI PipeHandler()
{
	const char* pipename = "\\\\.\\pipe\\byondcontrol13cmd";
	HANDLE hPipe;

	while (true)
	{
		WaitNamedPipeA(pipename, NMPWAIT_WAIT_FOREVER);

		hPipe = CreateFileA(pipename, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE, NULL, OPEN_EXISTING, 0, NULL);

		if (hPipe == INVALID_HANDLE_VALUE)
			continue;

		char buffer[250];
		memset(buffer, 0, sizeof(buffer));
		if (ReadFile(hPipe, buffer, sizeof(buffer), nullptr, NULL) == FALSE)
			continue;

		PipeExecute(std::string(buffer));

		DisconnectNamedPipe(hPipe);
	}

	return 1;
	/*
	HANDLE hPipe;

	while (TRUE)
	{
		hPipe = CreateNamedPipeA("\\\\.\\pipe\\byondcontrol13",
			PIPE_ACCESS_DUPLEX | PIPE_TYPE_BYTE | PIPE_READMODE_BYTE,
			PIPE_WAIT,
			1,
			1024 * 16,
			1024 * 16,
			NMPWAIT_USE_DEFAULT_WAIT,
			NULL);

		if (hPipe != NULL && hPipe != INVALID_HANDLE_VALUE)
		{
			printf("pipe OK\n");
			break;
		}

		printf("pipe failure %d\n", GetLastError());
		Sleep(1000);
	}

	char buffer[1024];
	DWORD dwRead;

	while (1)
	{
		if (ConnectNamedPipe(hPipe, NULL) != FALSE)
		{
			while (ReadFile(hPipe, buffer, sizeof(buffer) - 1, &dwRead, NULL) != FALSE)
			{
				buffer[dwRead] = '\0';
				PipeExecute(buffer);
			}
		}
		DisconnectNamedPipe(hPipe);
	}
	*/
}