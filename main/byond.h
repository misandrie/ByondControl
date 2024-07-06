#pragma once
#include <windows.h>

typedef long long Cid;

uintptr_t base = (uintptr_t)GetModuleHandleA("dreamseeker.exe");
uintptr_t dllBase = (uintptr_t)GetModuleHandleA("byondcore.dll");
uintptr_t winBase = (uintptr_t)GetModuleHandleA("byondwin.dll");
void* client = (void*)(*(int*)(base + 0x0009ca4c) + 156);
void* mainWindow = nullptr;
short* viewW1 = (short*)(dllBase + 0x0037a198);
short* viewW2 = (short*)(dllBase + 0x0037a180);
short* viewH1 = (short*)(dllBase + 0x0037a19c);
short* viewH2 = (short*)(dllBase + 0x00386ed0);
int* viewPixelOffset = (int*)(dllBase + 0x00360c1c);
int mapIconsList = dllBase + 0x003880c0;
int* sight = (int*)(*(int*)(dllBase + 0x003888d0) + 532);

enum class MouseParamsModifier: char {
    Default = 1,
    Ctrl = 8,
    Shift = 16,
    Alt = 32
};
DEFINE_ENUM_FLAG_OPERATORS(MouseParamsModifier)

struct MouseParams {
    long long Cid; // 0-8
    int unknown1; // 8-12
    int unknown2; // 12-16
    MouseParamsModifier modifiers; // 16-17 bitflags: ctrl 8, shift 16, alt 32, always 1
    char unknownc; // 17-18
    char unknownc2; // 18-19
    char unknownc3; // 19-20
    char* windName; // 20-24
    char* unknown4; // 24-28
    short tileX; // 28-30
    short tileY; // 30-32
    short pixelX; // 32-34
    short pixelY; // 34-36
    short pixelX2; // 36-38
    short pixelY2; // 38-40
    short relTileX; // 40-42
    short relTileY; // 42-44
    short relPixelX; // 44-46
    short relPixelY; // 46-48
    int unknown5; // 48-52
    int unknown6; // 52-56
    int unknown7; // 56-60
    int unknown8; // 60-64
};

#include "byondfuncs.h"
#include "byondobjects.h"