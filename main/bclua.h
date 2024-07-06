#pragma once
#include <windows.h>
#include <tchar.h>
#include <filesystem>
#include <fstream>

#include "dllmain.h"
#include "mapparser.h"
#include "curl/curl.h"

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"

extern "C"
{
#include "lua\include\lua.h"
#include "lua\include\lauxlib.h"
#include "lua\include\lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua/lua54.lib")
#endif

#include "Lua-ImGui/lib.hpp"

enum class ConsoleMessageType {
    error,
    warning,
    output
};
struct ConsoleMessage {
    std::string text;
    ConsoleMessageType type;
};

std::vector<lib::window> windows;
std::unordered_map<std::string, std::vector<int>> eventConnections;
std::vector<ConsoleMessage> outputList;
lua_State* l_G = luaL_newstate();
#include "Lua-ImGui/helper.hpp"
#include "Lua-ImGui/bind.hpp"

lua_State* luaMain;
bool luaConsoleEnabled = false;
bool showLuaWindows = true;

void LuaConsole() {
    if (!luaConsoleEnabled) {
        return;
    }
    ImGui::Begin("Lua Output");
    for (auto it = outputList.begin(); it != outputList.end(); ++it) {
        ImVec4 color;
        switch (it->type) {
        case ConsoleMessageType::error:
            color = ImVec4(1.0f, 0.0f, 0.0f, 1.0f);
            break;
        case ConsoleMessageType::warning:
            color = ImVec4(1.0f, 1.0f, 0.0f, 1.0f);
            break;
        default:
            color = ImVec4(1.0f, 1.0f, 1.0f, 1.0f);
            break;
        }
        ImGui::TextColored(color, "%s", it->text.c_str());
    }
    if (true && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()) {
        ImGui::SetScrollHereY(1.0f);
    }
    ImGui::End();
}

void ClearLuaConsole() {
    outputList.clear();
}

int OutputError(const char* l_error) {
    outputList.push_back(ConsoleMessage(std::string(l_error), ConsoleMessageType::error));
    return 0;
}

int CheckLua(int r) {
    if (r != LUA_OK) {
        const char* l_error = lua_tostring(luaMain, -1);
        OutputError(l_error);
    }
    return r;
}

void FireConnection(const char* name, int numArgs = 0) {
    auto it = eventConnections.find(name);
    if (it == eventConnections.end()) {
        OutputError("Attempted to fire connection that does not exist");
    }
    else {
        int top = lua_gettop(luaMain);
        for (auto ref = it->second.begin(); ref != it->second.end(); ++ref) {
            lua_rawgeti(luaMain, LUA_REGISTRYINDEX, *ref);
            // put the function beneath the arguments as per calling convention
            for (int i = 0; i < numArgs; ++i) {
                lua_pushvalue(luaMain, top - numArgs + 1 + i);
            }
            CheckLua(lua_pcall(luaMain, numArgs, 0, 0));
        }
        lua_pop(luaMain, numArgs);
    }
}

MouseParamsModifier GetModifierFromMouseOptions(lua_State* L, int optionsPos) {
    MouseParamsModifier modifiers = MouseParamsModifier::Default;
    lua_pushstring(L, "Alt");
    lua_gettable(L, -2);
    if (lua_toboolean(L, -1)) {
        modifiers |= MouseParamsModifier::Alt;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "Shift");
    lua_gettable(L, -2);
    if (lua_toboolean(L, -1)) {
        modifiers |= MouseParamsModifier::Shift;
    }
    lua_pop(L, 1);

    lua_pushstring(L, "Ctrl");
    lua_gettable(L, -2);
    if (lua_toboolean(L, -1)) {
        modifiers |= MouseParamsModifier::Ctrl;
    }
    lua_pop(L, 1);

    return modifiers;
}

void FireKeyBinding(int func) {
    //PIPELOG("firing %u", func);
    lua_rawgeti(luaMain, LUA_REGISTRYINDEX, func);
    CheckLua(lua_pcall(luaMain, 0, 0, 0));
}

int lua_MakeKeyBinding(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TNUMBER);
    luaL_checktype(L, 3, LUA_TBOOLEAN);
    luaL_checktype(L, 4, LUA_TFUNCTION);
    lua_pushvalue(L, 4);
    int ref = luaL_ref(L, LUA_REGISTRYINDEX);
    CreateLuaKeyBinding(
        lua_tostring(L, 1),
        lua_tointeger(L, 2),
        lua_toboolean(L, 3),
        ref,
        lua_toboolean(L, 5),
        lua_toboolean(L, 6),
        lua_toboolean(L, 7)
        );
    return 0;
}

void ClearConnections() {
    for (auto it = eventConnections.begin(); it != eventConnections.end(); ++it) {
        for (auto ref = it->second.begin(); ref != it->second.end(); ++ref) {
            luaL_unref(luaMain, LUA_REGISTRYINDEX, *ref);
        }
        it->second.clear();
    }
}

size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)// for curl
{
    size_t realsize = size * nmemb;
    ((std::string*)userp)->append((char*)contents, realsize);
    return realsize;
}

int lua_AddConnection(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    luaL_checktype(L, 2, LUA_TFUNCTION);

    const char* str = lua_tostring(L, 1);
    auto it = eventConnections.find(str);
    if (it == eventConnections.end()) {
        OutputError("Attempted to add connection to event that doesn't exist");
    }
    else {
        int ref = luaL_ref(L, LUA_REGISTRYINDEX);
        it->second.push_back(ref);
    }

    return 0;
}

int lua_ClearConnections(lua_State* L) {
    ClearConnections();
    return 0;
}

int lua_ClearKeyBindings(lua_State* L) {
    ClearLuaKeyBindings();
    return 0;
}

int lua_SetMoving(lua_State* L) {
    luaL_checktype(L, 1, LUA_TBOOLEAN);
    autoMoveEnabled = lua_toboolean(L, 1);
    return 0;
}

int lua_SetMoveTarget(lua_State* L) {
    luaL_checktype(L, 1, LUA_TNUMBER);
    luaL_checktype(L, 2, LUA_TNUMBER);
    autoMoveX = (short)lua_tointeger(L, 1);
    autoMoveY = (short)lua_tointeger(L, 2);
    return 0;
}

int lua_DoCommand(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    ParseCommand(client, client, lua_tostring(L, 1), 0, 1);
    return 0;
}

int lua_RespondPrompt(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    ParseCommand(client, client, (std::string(".prompt ") + lua_tostring(L, 1)).c_str(), 1, 0);
    return 0;
}

int lua_KeyDown(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    ParseCommand(client, client, (std::string("KeyDown \"") + lua_tostring(L, 1) + "\"").c_str(), 0, 0);
    return 0;
}

int lua_KeyUp(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    ParseCommand(client, client, (std::string("KeyUp \"") + lua_tostring(L, 1) + "\"").c_str(), 0, 0);
    return 0;
}

int lua_KeyStroke(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    ParseCommand(client, client, (std::string("KeyDown \"") + lua_tostring(L, 1) + "\"").c_str(), 0, 0);
    ParseCommand(client, client, (std::string("KeyUp \"") + lua_tostring(L, 1) + "\"").c_str(), 0, 0);
    return 0;
}

int lua_MouseClick(lua_State* L) {
    if (lua_type(L, 1) == LUA_TTABLE) {
        luaL_checktype(L, 1, LUA_TTABLE);

        lua_pushstring(L, "Id");
        lua_gettable(L, 1);
        long long cid = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_pushstring(L, "X");
        lua_gettable(L, 1);
        short x = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        lua_pushstring(L, "Y");
        lua_gettable(L, 1);
        short y = luaL_checkinteger(L, -1);
        lua_pop(L, 1);

        // optional modifiers
        MouseParamsModifier modifiers = MouseParamsModifier::Default;
        if (lua_gettop(L) > 1) {
            luaL_checktype(L, 2, LUA_TTABLE);

            modifiers = GetModifierFromMouseOptions(L, 2);
        }

        ClickOn(cid, x, y, modifiers);
    }
    else {
        luaL_checktype(L, 1, LUA_TNUMBER);
        luaL_checktype(L, 2, LUA_TNUMBER);
        luaL_checktype(L, 3, LUA_TNUMBER);

        long long cid = lua_tointeger(L, 1);
        short x = lua_tointeger(L, 2);
        short y = lua_tointeger(L, 3);

        // optional modifiers
        MouseParamsModifier modifiers = MouseParamsModifier::Default;
        if (lua_gettop(L) > 3) {
            luaL_checktype(L, 4, LUA_TTABLE);

            modifiers = GetModifierFromMouseOptions(L, 4);
        }

        ClickOn(cid, x, y, modifiers);
    }
    return 0;
}

int lua_Print(lua_State* L) {
    int count = lua_gettop(L);
    if (count == 0) {
        // nothing to be printed
        return 0;
    }
    std::string outStr = std::string();
    for (int i = 1; i <= count; ++i) {
        lua_getglobal(L, "tostring");
        lua_pushvalue(L, i);
        lua_call(L, 1, 1);
        const char* str = lua_tostring(L, -1);
        lua_pop(L, 1);
        if (str != NULL) {
            outStr += ' ';
            outStr += str;
        }
    }
    outputList.push_back(ConsoleMessage(outStr, ConsoleMessageType::output));

    return 0;
}

int lua_GetMobPosition(lua_State* L) {
    lua_pushnumber(L, getPlayerX());
    lua_pushnumber(L, getPlayerY());
    return 2;
}

#define LUA_BYONDMOUSEPARAMS "ByondMouseParams"
int byondMouseParams__index(lua_State* L) {
    MouseParams* params = *(MouseParams**)luaL_checkudata(L, 1, LUA_BYONDMOUSEPARAMS);
    const char* index = luaL_checkstring(L, 2);
    lua_settop(L, 0);
    if (strcmp(index, "HitId") == 0) {
        lua_pushinteger(L, params->Cid);
        return 1;
    }
    if (strcmp(index, "HitX") == 0) {
        lua_pushinteger(L, params->tileX);
        return 1;
    }
    if (strcmp(index, "HitY") == 0) {
        lua_pushinteger(L, params->tileX);
        return 1;
    }
    if (strcmp(index, "HitPixelX") == 0) {
        lua_pushinteger(L, params->pixelX);
        return 1;
    }
    if (strcmp(index, "HitPixelY") == 0) {
        lua_pushinteger(L, params->pixelY);
        return 1;
    }
    if (strcmp(index, "Alt") == 0) {
        lua_pushboolean(L, (int)(params->modifiers & MouseParamsModifier::Alt));
        return 1;
    }
    if (strcmp(index, "Shift") == 0) {
        lua_pushboolean(L, (int)(params->modifiers & MouseParamsModifier::Shift));
        return 1;
    }
    if (strcmp(index, "Ctrl") == 0) {
        lua_pushboolean(L, (int)(params->modifiers & MouseParamsModifier::Ctrl));
        return 1;
    }
    return 0;
}

int byondMouseParams__newindex(lua_State* L) {
    MouseParams* params = *(MouseParams**)luaL_checkudata(L, 1, LUA_BYONDMOUSEPARAMS);
    const char* index = luaL_checkstring(L, 2);
    if (strcmp(index, "HitId") == 0) {
        params->Cid = luaL_checknumber(L, 3);
        return 0;
    }
    if (strcmp(index, "HitX") == 0) {
        short newTileX = (short)luaL_checknumber(L, 3);
        params->relTileX += newTileX - params->tileX;
        params->tileX = newTileX;
        return 0;
    }
    if (strcmp(index, "HitY") == 0) {
        short newTileY = (short)luaL_checknumber(L, 3);
        params->relTileY += newTileY - params->tileY;
        params->tileY = newTileY;
        return 0;
    }
    if (strcmp(index, "HitPixelX") == 0) {
        short newPixelX = (short)luaL_checknumber(L, 3);
        params->relPixelX += newPixelX - params->pixelX;
        params->pixelX = newPixelX;
        params->pixelX2 = newPixelX;
        return 0;
    }
    if (strcmp(index, "HitPixelY") == 0) {
        short newPixelY = (short)luaL_checknumber(L, 3);
        params->relPixelY += newPixelY - params->pixelY;
        params->pixelY = newPixelY;
        params->pixelY2 = newPixelY;
        return 0;
    }
    if (strcmp(index, "Alt") == 0) {
        luaL_checktype(L, 3, LUA_TBOOLEAN);
        if (((int)(params->modifiers & MouseParamsModifier::Alt) != 0) != lua_toboolean(L, 3)) {
            params->modifiers ^= MouseParamsModifier::Alt;
        }
        return 0;
    }
    if (strcmp(index, "Shift") == 0) {
        luaL_checktype(L, 3, LUA_TBOOLEAN);
        if (((int)(params->modifiers & MouseParamsModifier::Shift) != 0) != lua_toboolean(L, 3)) {
            params->modifiers ^= MouseParamsModifier::Shift;
        }
        return 0;
    }
    if (strcmp(index, "Ctrl") == 0) {
        luaL_checktype(L, 3, LUA_TBOOLEAN);
        if (((int)(params->modifiers & MouseParamsModifier::Ctrl) != 0) != lua_toboolean(L, 3)) {
            params->modifiers ^= MouseParamsModifier::Ctrl;
        }
        return 0;
    }
    return 0;
}

void createByondMouseParamsMeta(lua_State* L) {
    if (luaL_newmetatable(L, LUA_BYONDMOUSEPARAMS)) {
        // initialize metatable
        //PIPELOG("%u", lua_gettop(L));
        lua_pushstring(L, "__index");
        lua_pushcfunction(L, byondMouseParams__index);
        lua_rawset(L, -3);

        lua_pushstring(L, "__newindex");
        lua_pushcfunction(L, byondMouseParams__newindex);
        lua_rawset(L, -3);
    }
    lua_pop(L, 1);
}

void FireMouseConnection(const char* name, MouseParams* params) {
    MouseParams** ptr = (MouseParams**)lua_newuserdatauv(luaMain, sizeof(params), 0);
    *ptr = params;
    luaL_setmetatable(luaMain, LUA_BYONDMOUSEPARAMS);
    FireConnection(name, 1);
}

void FireScriptConnection(char* script) {
    lua_pushstring(luaMain, script);
    FireConnection("JSInvoke", 1);
}

void FireCommandConnection(const char* command, char isPrompt) {
    lua_pushstring(luaMain, command);
    lua_pushboolean(luaMain, isPrompt);
    FireConnection("CommandRun", 2);
}

//#define LUA_BYONDOBJECT "ByondObject"
//int byondObjectIndex(lua_State* L) {
//    byondtile* object = (byondtile*)luaL_checkudata(L, 1, LUA_BYONDOBJECT);
//    char* index = luaL_checkstring(L, 2);
//    return 0;
//}
//void createByondObjectMeta(lua_State* L) {
//    luaL_newmetatable(L, LUA_BYONDOBJECT);
//    lua_pushstring(L, "__index");
//    lua_pushcfunction(L, byondObjectIndex);
//    lua_rawset(L, -3);
//}
void AddObject(lua_State* L, int ptr) {
    lua_createtable(L, 0, 5);

    lua_pushstring(L, "Name");
    lua_pushstring(L, GetCidName(client, client, *(long long*)(ptr + 124)));
    lua_settable(L, -3); //

    lua_pushstring(L, "Plane");
    lua_pushstring(L, GetCidName(client, client, *(short*)(ptr + 116)));
    lua_settable(L, -3); //

    lua_pushstring(L, "X");
    lua_pushinteger(L, *(short*)(ptr + 28));
    lua_settable(L, -3); //

    lua_pushstring(L, "Y");
    lua_pushinteger(L, *(short*)(ptr + 30));
    lua_settable(L, -3); //

    lua_pushstring(L, "Id");
    lua_pushinteger(L, *(long long*)(ptr + 124));
    lua_settable(L, -3); //

    lua_pushstring(L, "IsMob");
    lua_pushboolean(L, (*(char*)(ptr + 124) == 1) ? true : false);
    lua_settable(L, -3); //

    // results in one table on top of stack
}

int lua_GetObjects(lua_State* L) {
    short numPlanes = *(short*)(mapIconsList + 4) - 1;
    lua_createtable(L, numPlanes, 0);
    for (int p = 0; p < numPlanes; p++)
    {
        int planePtr = *(int*)mapIconsList + p * 212;
        short start = *(short*)(planePtr + 162);
        short end = *(short*)(planePtr + 164) - 1;
        lua_pushinteger(L, p + 1);
        AddObject(L, planePtr);

        lua_pushstring(L, "Children");
        lua_createtable(L, end - start, 0);
        for (int j = start; j < end; j++)
        {
            lua_pushinteger(L, j - start + 1);
            AddObject(L, *(int*)mapIconsList + j * 212);
            lua_settable(L, -3); //
            // add to children table
        }

        lua_settable(L, -3); // add children table to object

        lua_settable(L, -3); // add object to list
    }
    return 1;
}


int lua_RunJavascript(lua_State* L) {
    luaL_checktype(L, 1, LUA_TSTRING);
    RunJavascript((char*)lua_tostring(L, 1));
    return 0;
}

int lua_GetServerIP(lua_State* L) {
    char* ip = GetServerIP(client, 0, nullptr);
    lua_pushstring(L, ip);
    return 1;
}
int lua_sleep(lua_State* L) {
    int m = static_cast<int> (luaL_checknumber(L, 1));
    Sleep(m);
    return 0;
}

int lua_tick(lua_State* L) {
    long long t = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();;
    lua_pushinteger(L, t);
    return 1;
}


int lua_GetServerPort(lua_State* L) {
    int port = GetServerPort(client, 0);
    lua_pushinteger(L, port);
    return 1;
}

int lua_HttpGet(lua_State* L)
{
    luaL_checktype(L, 1, LUA_TSTRING);

    const char* url = lua_tostring(L, 1);
    std::string buffer;

    CURL* req = curl_easy_init();
    CURLcode res;
    if (req)
    {
        curl_easy_setopt(req, CURLOPT_URL, url);
        curl_easy_setopt(req, CURLOPT_FOLLOWLOCATION, 1L);
        curl_easy_setopt(req, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(req, CURLOPT_WRITEDATA, &buffer);
        res = curl_easy_perform(req);
        if (res != CURLE_OK)
        {
            luaL_error(L, "curl failed: %s\n", curl_easy_strerror(res));
        }
    }
    curl_easy_cleanup(req);
    lua_pushstring(L, buffer.c_str());
    return 1;
}

void RunLua(char* data) {
    CheckLua(luaL_dostring(luaMain, data));

    delete[] data;
}

#define HFILEBUFFERSIZE 2048
#define FILEBUFFERSIZE 512
void LoadLuaFile() {
    try {
        OPENFILENAME open = { 0 };
        ZeroMemory(&open, sizeof(OPENFILENAME));

        wchar_t hFileBuffer[HFILEBUFFERSIZE];
        open.lStructSize = sizeof(OPENFILENAME);
        open.lpstrFilter = _T("lua source files (*.lua, *.txt)\0*.lua;*.txt\0\0");
        open.nFilterIndex = 1;
        open.lpstrFile = hFileBuffer;
        open.nMaxFile = HFILEBUFFERSIZE;
        open.lpstrFile[0] = '\0';
        open.lpstrFileTitle = NULL;
        open.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

        BOOL selected = GetOpenFileNameW(&open);
        if (selected) {
            FILE* file;
            // TODO: _T (sus man is mad and peeing himself)
            if (_wfopen_s(&file, open.lpstrFile, _T("rb")) == 0 && file) {
                fseek(file, 0, SEEK_END);
                long fEnd = ftell(file);
                if (fEnd == -1) { // returns -1 if error
                    PIPELOG("Error getting length of file.");
                }
                else {
                    //PIPELOG("File length: %u", fEnd);
                    rewind(file);
                    size_t fSize = fEnd;
                    char* data = new char[fSize + 1];
                    fread(data, fSize, 1, file);
                    data[fSize] = '\0';

                    RunLua(data);
                }
                fclose(file);
            }
        }
        else {
            PIPELOG("Couldn't open file.");
            DWORD err = CommDlgExtendedError();
            if (err != 0) {
                PIPELOG("%u", err);
            }
        }
    }
    catch (const std::exception& e)
    {
        PIPELOG("Error opening file dialog. %s", e.what());
    }
}

void LuaWindows() {
    for (std::vector<lib::window>::iterator it = windows.begin(); it != windows.end(); ++it) {
        (*it).render();
    }
}

void AutoExecute() {
    try
    {
        std::string path = bcpath;
        path += "\\autoexec";

        if (!std::filesystem::exists(path.c_str()))
        {
            PIPELOG("autoexec folder is missing");
            return;
        }

        for (const auto& entry : std::filesystem::directory_iterator(path))
        {
            if (entry.is_directory())
                continue;

            FILE* file;
            if (_wfopen_s(&file, entry.path().wstring().c_str(), L"rb") == 0 && file) {
                fseek(file, 0, SEEK_END);
                size_t len = ftell(file);
                rewind(file);
                char* data = new char[len + 1];
                fread(data, len, 1, file);
                data[len] = '\0';
                fclose(file);
                RunLua(data);
            }
        }
    }
    catch (const std::exception& e)
    {
        PIPELOG("error with autoexec");
        PIPELOG("%s", e.what());
    }
}

static const luaL_Reg loadedlibs[] = {
  {LUA_GNAME, luaopen_base},
  //{LUA_LOADLIBNAME, luaopen_package},
  {LUA_COLIBNAME, luaopen_coroutine},
  {LUA_TABLIBNAME, luaopen_table},
  //{LUA_IOLIBNAME, luaopen_io},
  //{LUA_OSLIBNAME, luaopen_os},
  {LUA_STRLIBNAME, luaopen_string},
  {LUA_MATHLIBNAME, luaopen_math},
  {LUA_UTF8LIBNAME, luaopen_utf8},
  {LUA_DBLIBNAME, luaopen_debug},
  {NULL, NULL}
};


LUALIB_API void opensafelibs(lua_State* L) {
    const luaL_Reg* lib;
    /* "require" functions from 'loadedlibs' and set results to global table */
    for (lib = loadedlibs; lib->func; lib++) {
        luaL_requiref(L, lib->name, lib->func, 1);
        lua_pop(L, 1);  /* remove lib */
    }
}

void LuaInit() {
    luaMain = l_G;
    eventConnections["Tick"] = std::vector<int>();
    eventConnections["MouseLClick"] = std::vector<int>();
    eventConnections["MouseLUp"] = std::vector<int>();
    // TODO: Mouse right button events
    eventConnections["JSInvoke"] = std::vector<int>();
    eventConnections["CommandRun"] = std::vector<int>();

    //luaL_openlibs(luaMain);
    opensafelibs(luaMain);


    lua_register(luaMain, "AddConnection", lua_AddConnection);
    lua_register(luaMain, "ClearConnections", lua_ClearConnections);
    lua_register(luaMain, "SetMoving", lua_SetMoving);
    lua_register(luaMain, "SetMoveTarget", lua_SetMoveTarget);
    lua_register(luaMain, "DoCommand", lua_DoCommand);
    lua_register(luaMain, "RespondPrompt", lua_RespondPrompt);
    lua_register(luaMain, "GetMobPosition", lua_GetMobPosition);
    lua_register(luaMain, "KeyDown", lua_KeyDown);
    lua_register(luaMain, "KeyUp", lua_KeyUp);
    lua_register(luaMain, "KeyStroke", lua_KeyStroke);
    lua_register(luaMain, "MouseClick", lua_MouseClick);
    lua_register(luaMain, "GetObjects", lua_GetObjects);
    lua_register(luaMain, "print", lua_Print);
    lua_register(luaMain, "sleep", lua_sleep);
    lua_register(luaMain, "tick", lua_tick);
    lua_register(luaMain, "MakeKeyBinding", lua_MakeKeyBinding);
    lua_register(luaMain, "ClearKeyBindings", lua_ClearKeyBindings);
    lua_register(luaMain, "RunJavascript", lua_RunJavascript);
    lua_register(luaMain, "GetServerIP", lua_GetServerIP);
    lua_register(luaMain, "GetServerPort", lua_GetServerPort);
    lua_register(luaMain, "HttpGet", lua_HttpGet);

    LuaRegisterMapParser(luaMain);

    createByondMouseParamsMeta(luaMain);

    lua_bind::init();

    AutoExecute();
}