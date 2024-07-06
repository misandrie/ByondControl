#define _CRT_SECURE_NO_WARNINGS
#include <windows.h>
#include <string>
#include <winsock.h>
#include "authlib.h"

int WebRequest(std::string& res, std::string host, std::string path)
{
	WSADATA wsaData;
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
		return 1;
	}

	SOCKET sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	struct hostent* remoteHost = gethostbyname(host.c_str());

	SOCKADDR_IN SockAddr;
	SockAddr.sin_port = htons(80);
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_addr.s_addr = *((unsigned long*)remoteHost->h_addr);

	if (connect(sock, (SOCKADDR*)(&SockAddr), sizeof(SockAddr)) != 0) {
		return 2;
	}

	std::string req = "GET " + path + " HTTP/1.1\r\nHost: austism.net\r\nConnection: close\r\n\r\n";

	send(sock, req.c_str(), strlen(req.c_str()), 0);

	char buffer[10000];
	while (recv(sock, buffer, sizeof(buffer), 0) > 0) {
		int i = 0;
		while (buffer[i] >= 32 || buffer[i] == '\n' || buffer[i] == '\r') {
			res += buffer[i];
			i += 1;
		}
	}

	closesocket(sock);
	WSACleanup();

	return 0;
}

bool GetHWID(std::string& hwid)
{
	HW_PROFILE_INFOA hwProfileInfo;
	if (!GetCurrentHwProfileA(&hwProfileInfo))
		return false;

	hwid = hwProfileInfo.szHwProfileGuid;

	return true;
}

int CheckKey(char* key)
{
#ifdef KEYSYSTEM_ENABLED
	std::string hwid;

	if (!GetHWID(hwid))
	{
		return -1;
	}

	std::string response;
	if (WebRequest(response, "austism.net", "/byondcontrol/check.php?key=" + std::string(key) + "&hwid=" + hwid) != 0)
	{
		return -2;
	}

	if (response.find(std::string("good key")) != std::string::npos) {
		return KEYSYSTEM_RES_VALID;
	}
	else if (response.find(std::string("bad key")) != std::string::npos) {
		return KEYSYSTEM_RES_INVALID;
	}
	else if (response.find(std::string("bad hwid")) != std::string::npos) {
		return KEYSYSTEM_RES_BADHWID;
	}
	MessageBoxA(0, response.c_str(), "w", MB_OK);
	return 0;
#else
	return KEYSYSTEM_RES_VALID;
#endif
}

bool GetSavedKey(char*& key)
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Dantom\\BYOND", NULL, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		char value[256];
		DWORD count = sizeof(value);
		DWORD dataType = REG_SZ;
		if (RegQueryValueExA(hKey, "byondcontrolkey", NULL, &dataType, (LPBYTE)&value, &count) == ERROR_SUCCESS) {
			value[count - 1] = '\0';
			strcpy(key, value);
			RegCloseKey(hKey);
			return true;
		}
	}
	RegCloseKey(hKey);
	return false;
}

bool SaveKey(char* key)
{
	HKEY hKey;
	if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Dantom\\BYOND", NULL, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
		if (RegSetValueExA(hKey, "byondcontrolkey", 0, REG_SZ, (LPBYTE)key, strlen(key) + 1) == ERROR_SUCCESS) {
			RegCloseKey(hKey);
			return true;
		}
	}
	RegCloseKey(hKey);
	return false;
}

bool KeySysEnabled()
{
#ifdef KEYSYSTEM_ENABLED
	return true;
#else
	return false;
#endif
}