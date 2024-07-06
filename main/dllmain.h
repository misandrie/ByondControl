#pragma once
#include "byond.h"
#include <unordered_set>

extern bool autoMoveEnabled;
extern bool autoMoveCoords;
extern short autoMoveX;
extern short autoMoveY;
extern std::string bcpath;

void ClickOn(long long cid, short tileX, short tileY, MouseParamsModifier modifiers = MouseParamsModifier::Default);

void RenderImGui();

void FireKeyBinds(int, bool, bool, bool, bool);
void CreateLuaKeyBinding(const char* name, int key, bool isClick, int func, bool ctrl, bool shift, bool alt);
void ClearLuaKeyBindings();

void RunJavascript(char* script);

bool ShouldChangeHash();