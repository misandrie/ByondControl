#pragma once
#include <d3d9.h>
#include <windows.h>

#pragma region DIRECT3D
typedef long(__stdcall* Reset)(LPDIRECT3DDEVICE9, D3DPRESENT_PARAMETERS*);
static Reset oReset = NULL;

typedef long(__stdcall* EndScene)(LPDIRECT3DDEVICE9);
static EndScene oEndScene = NULL;

typedef long(__stdcall* BeginScene)(LPDIRECT3DDEVICE9);
static BeginScene oBeginScene = NULL;

typedef long(__stdcall* Present)(LPDIRECT3DDEVICE9, const RECT*, const RECT*, HWND, const RGNDATA*);
static Present oPresent = NULL;

typedef HRESULT(__stdcall* Clear)(LPDIRECT3DDEVICE9, DWORD, const D3DRECT*, DWORD, D3DCOLOR, float, DWORD);
static Clear oClear = NULL;
#pragma endregion

typedef int(__fastcall* PIsByondMember)(void*, void*, int*);
static PIsByondMember IsByondMember = (PIsByondMember)(GetProcAddress((HMODULE)dllBase, "?IsByondMember@DungClient@@QAE_NPAJ@Z"));

typedef int(__fastcall* PGetByondVersion)(void*);
static PGetByondVersion GetByondVersion = (PGetByondVersion)(GetProcAddress((HMODULE)dllBase, "?GetByondVersion@ByondLib@@QAEJXZ"));

typedef int(__fastcall* PGetByondBuild)(void*);
static PGetByondBuild GetByondBuild = (PGetByondBuild)(GetProcAddress((HMODULE)dllBase, "?GetByondBuild@ByondLib@@QAEJXZ"));

typedef long long* (__stdcall* PGetDefaultStatCid)(long long*);
static PGetDefaultStatCid GetDefaultStatCid = (PGetDefaultStatCid)(GetProcAddress((HMODULE)dllBase, "?GetDefaultStatCid@DSStat@@QAE?AUCid@@XZ"));

typedef char* (__fastcall* PGetCidName)(void*, void*, long long);
static PGetCidName GetCidName = (PGetCidName)(GetProcAddress((HMODULE)dllBase, "?GetCidName@DungClient@@QAEPBDUCid@@@Z"));


typedef int(__fastcall* PGenMouseMoveCommand)(void*, void*, MouseParams*);
static PGenMouseMoveCommand GenMouseMoveCommand = (PGenMouseMoveCommand)(GetProcAddress((HMODULE)dllBase, "?GenMouseMoveCommand@DungClient@@QAEHABUMouseParams@@@Z"));

typedef int(__fastcall* PGenClickCommand)(void*, void*, MouseParams*);
static PGenClickCommand GenClickCommand = (PGenClickCommand)(GetProcAddress((HMODULE)dllBase, "?GenClickCommand@DungClient@@QAEHABUMouseParams@@@Z"));

typedef int(__fastcall* PGenMouseDownCommand)(void*, void*, MouseParams*);
static PGenMouseDownCommand GenMouseDownCommand = (PGenMouseDownCommand)(GetProcAddress((HMODULE)dllBase, "?GenMouseDownCommand@DungClient@@QAEHABUMouseParams@@@Z"));

typedef int(__fastcall* PGenMouseUpCommand)(void*, void*, MouseParams*);
static PGenMouseUpCommand GenMouseUpCommand = (PGenMouseUpCommand)(GetProcAddress((HMODULE)dllBase, "?GenMouseUpCommand@DungClient@@QAEHABUMouseParams@@@Z"));

typedef char(__cdecl* PMapIcons)(char*, int*, char, char);
static PMapIcons MapIcons = (PMapIcons)(dllBase + 0x0002c840);

typedef char(__fastcall* PGetCidMouseOpacity)(void*, void*, long long);
static PGetCidMouseOpacity GetCidMouseOpacity = (PGetCidMouseOpacity)(GetProcAddress((HMODULE)dllBase, "?GetCidMouseOpacity@DungClient@@QAECUCid@@@Z"));


typedef char(__fastcall* PParseCommand)(void*, void*, const char*, char, char);
static PParseCommand ParseCommand = (PParseCommand)(GetProcAddress((HMODULE)dllBase, "?ParseCommand@DungClient@@QAEEPBDEE@Z"));


//typedef void(__fastcall* PmapWindowTick)(void*, void*, char);
//static PmapWindowTick mapWindowTick = (PmapWindowTick)(base + 0x00008040);

typedef void*(__fastcall* PClientMapCtrl)(void*, void*, int, void*, void*);
static PClientMapCtrl clientMapCtrl = (PClientMapCtrl)(base + 0x7910);

//typedef void (__fastcall* PD3D)(void*, void*, char);
//static PD3D D3D = (PD3D)(winBase + 0xc0e50);


typedef void(__cdecl* PTick)();
static PTick Tick = (PTick)(dllBase + 0x00074c80);

typedef char* (__cdecl* Pmd5)(char*);
static Pmd5 md5 = (Pmd5)(GetProcAddress((HMODULE)dllBase, "?md5@@YAPBDPBD@Z"));

typedef int(__fastcall* PInvokeScript)(void*, void*, char*, char*);
static PInvokeScript InvokeScript = (PInvokeScript)(GetProcAddress((HMODULE)winBase, "?InvokeScript@CVHTMLCtrl@@QAEHPBDPAUDMString@@@Z"));

typedef void*(__fastcall* PhtmlCtrl)(void*, void*, void*);
static PhtmlCtrl htmlCtrl = (PhtmlCtrl)(GetProcAddress((HMODULE)winBase, "??0CVHTMLCtrl@@QAE@PAVCWnd@@@Z"));

typedef void (__fastcall* PhtmlCtrlDestructor)(void*, void*);
static PhtmlCtrlDestructor htmlCtrlDestructor = (PhtmlCtrlDestructor)(GetProcAddress((HMODULE)winBase, "??1CVHTMLCtrl@@UAE@XZ"));

typedef void* (__fastcall* PDMString)(int*, int, const char*);
static PDMString DMString = (PDMString)(GetProcAddress((HMODULE)dllBase, "??0DMString@@QAE@PBD@Z"));

//typedef void (__fastcall* PTestTest)(void*, void*, int, int, void*);
//static PTestTest testtest = (PTestTest)(base + 0x9120);

//typedef int(__fastcall* PSight)(int);
//static PSight sight = (PSight)(dllBase + 0x249960);

/*typedef char(__fastcall* PObscure)(void* pThis, void* unused, short a, short b, short c, short d, short e, short f, short g, char h, char i, char j, char k, char l);
static PObscure obscure = (PObscure)(dllBase + 0x249830);

typedef int(__cdecl* PObscure2)(int a);
static PObscure2 obscure2 = (PObscure2)(dllBase + 0x0327a0);

typedef int(__fastcall* Pnetworkmsg)(void* pThis, void* unused, short* a, int* pointer);
static Pnetworkmsg networkmsg = (Pnetworkmsg)(dllBase + 0x0265850);

typedef int(__cdecl* Pviewmsg)(int a);
static Pviewmsg viewmsg = (Pviewmsg)(dllBase + 0x044e00);*/

typedef char* (__fastcall* PGetServerIP)(void* DungClient, void*, char*);
static PGetServerIP GetServerIP = (PGetServerIP)(GetProcAddress((HMODULE)dllBase, "?GetServerIP@DungClient@@QAEPBDPBD@Z"));

typedef int (__fastcall* PGetServerPort)(void* DungClient, void*);
static PGetServerPort GetServerPort = (PGetServerPort)(GetProcAddress((HMODULE)dllBase, "?GetRemoteServerPort@DungClient@@QAEKXZ"));
