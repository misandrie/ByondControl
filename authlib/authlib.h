#pragma once

#ifdef AUTHLIB_EXPORTS
#define AUTHLIB_API __declspec(dllexport)
#else
#define AUTHLIB_API __declspec(dllimport)
#endif

#ifndef DEBUG
#define KEYSYSTEM_ENABLED
#endif

#define KEYSYSTEM_RES_VALID    1
#define KEYSYSTEM_RES_INVALID  2
#define KEYSYSTEM_RES_BADHWID  3

extern "C" AUTHLIB_API int CheckKey(char* key);
extern "C" AUTHLIB_API bool SaveKey(char* key);
extern "C" AUTHLIB_API bool GetSavedKey(char*& key);
extern "C" AUTHLIB_API bool KeySysEnabled();