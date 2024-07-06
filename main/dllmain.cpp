#include "dllmain.h"
#include "bclua.h"
#include "mapparser.h"

#include "pch.h"
#include <stdio.h>
#include <windows.h>
#include <iostream>
#include <detours.h>
#include <cstdint>
#include <vector>
#include <algorithm>
#include <d3d9.h>
#include <tchar.h>
#include <chrono>
#include <thread>
#include <list>
#include <map>
#include <unordered_map>

#include "imgui.h"
#include "imgui_impl_dx9.h"
#include "imgui_impl_win32.h"
#include "byond.h"
#include "cidspoofer.h"
#include "uimain.h"
#include "authlib.h"

#include <commdlg.h>
#include "stdio.h"

typedef void(*PBindFunc)();

struct KeyBind {
    bool disabled;
    bool ctrl;
    bool shift;
    bool alt;
    bool isClick;
    int key;
    int func;
    bool isLuaBinding;
};

enum class codebase: int {
    Bee,
    Tg,
    Yog
};

codebase currentServer = codebase::Tg;
std::unordered_map<std::string, KeyBind> bindsList;
const std::string* editingBind = nullptr;
bool eatInput = true;

long long latestMouseCid = 0;
char targetTile[128];
bool targetMobs = true;

bool autoMoveEnabled = false;
short autoMoveX = 0;
short autoMoveY = 0;
short targetX = 0;
short targetY = 0;
long long pathfindLastCheck = 0LL;

bool murderboneEnabled = false;
bool mobWhitelistEnabled = false;
std::unordered_set<long long> whitelist;
std::unordered_set<long long> blacklist;

bool autoClickEnabled = false;
int autoClickMaxRange = 2;
int autoClickSpeed = 25;
long long autoClickLastClickTime = 0LL;

bool fullbrightEnabled = false;
int brightnessAlpha = 0;
bool hideOverlays = false;
char hideObjs = 0;
int detailAlpha = 0;
bool resetViewPixelOffset = false;
bool objectsList = true;

bool autoFlipper = false;
bool autoScream = false;
bool autoLaugh = false;
long long lastFlipTime = 0LL;
long long lastScreamTime = 0LL;
long long lastLaughTime = 0LL;

bool higherView = false;
int extraW = 4;
int extraH = 4;
short currentW = 0;
short currentH = 0;
short realW = 0;
short realH = 0;

#ifdef _DEBUG
int DEBUG_hideLayer = -1;
bool DEBUG_shouldHideLayer = false;
int DEBUG_hidePlane = -1;
bool DEBUG_shouldHidePlane = false;
#endif

int settingsVal = 0;

std::vector<void*> browsers;

std::string bcpath;

uintptr_t FindAddy(uintptr_t ptr, std::vector<unsigned int> offsets)
{
    uintptr_t addr = ptr;
    for (unsigned int i = 0; i < offsets.size(); ++i)
    {
        addr = *(uintptr_t*)addr;
        addr += offsets[i];
    }
    return addr;
}

bool ShouldMakeUI() {
    return (settingsVal & 2) == 0;
}

bool ShouldChangeHash() {
    return (settingsVal & 1) == 0;
}

void FireKeyBinds(int key, bool isClick, bool ctrl, bool shift, bool alt) {
    if (editingBind != nullptr) {
        if (key == VK_LCONTROL || key == VK_LSHIFT || key == VK_LMENU) {
            return;
        }
        KeyBind& bind = bindsList.at(*editingBind);
        bind.disabled = key == VK_ESCAPE;
        bind.key = key;
        bind.isClick = isClick;
        bind.ctrl = ctrl;
        bind.shift = shift;
        bind.alt = alt;
        editingBind = nullptr;
    }
    else {
        for (auto it = bindsList.begin(); it != bindsList.end(); ++it) {
            KeyBind bind = it->second;
            if ((bind.ctrl && ctrl || !bind.ctrl)
                && (bind.shift && shift || !bind.shift)
                && (bind.alt && shift || !bind.alt)
                && (bind.isClick == isClick)
                && (!bind.disabled)
                && (bind.key == key)) {
                if (bind.isLuaBinding) {
                    FireKeyBinding(bind.func);
                }
                else {
                    (*(PBindFunc)bind.func)();
                }
            }
        }
    }
}

void CreateLuaKeyBinding(const char* name, int key, bool isClick, int func, bool ctrl, bool shift, bool alt) {
    struct KeyBind bind;
    bind.disabled = false;
    bind.ctrl = ctrl;
    bind.shift = shift;
    bind.alt = alt;
    bind.isClick = isClick;
    bind.key = key;
    bind.func = func;
    //PIPELOG("create %u", func);
    bind.isLuaBinding = true;
    bindsList[name] = bind;
}

void ClearLuaKeyBindings() {
    auto it = bindsList.begin();
    while (it != bindsList.end()) {
        KeyBind bind = it->second;
        if (bind.isLuaBinding) {
            bindsList.erase(it++);
        }
        else {
            ++it;
        }
    }
}

#define PIXEL_X 16
#define PIXEL_Y 16
void ClickOn(long long cid, short tileX, short tileY, MouseParamsModifier modifiers) {
    MouseParams params = MouseParams();
    params.Cid = cid;
    params.modifiers = modifiers;
    params.unknownc = 1;
    params.windName = (char*)"mapwindow.map";
    params.tileX = tileX; params.relTileX = tileX - getPlayerX() + realW / 2 + 1;
    params.tileY = tileY; params.relTileY = tileY - getPlayerY() + realH / 2 + 1;
    params.pixelX = PIXEL_X; params.pixelX2 = PIXEL_X; params.relPixelX = PIXEL_X;
    params.pixelY = PIXEL_Y; params.pixelY2 = PIXEL_Y; params.relPixelY = PIXEL_Y;
    GenMouseDownCommand(client, client, &params);
    GenClickCommand(client, client, &params);
    GenMouseUpCommand(client, client, &params);
}

void FixMouseParams(MouseParams* params) {
    if (higherView) {
        params->relTileX -= (currentW - realW) / 2;
        params->relTileY -= (currentH - realH) / 2;
    }
}

bool WhitelistBlacklistCheck(long long cid) {
    if (blacklist.count(cid) != 0) { // they are in the blacklist table -> don't let them be hit
        return false;
    }
    if (mobWhitelistEnabled) { // whitelist is turned on
        return whitelist.count(cid) != 0; // they are in the whitelist table -> hit them, otherwise don't
    }
    return true; // hit them. not in blacklist and whitelist disabled
}

void __fastcall DoAutoClick()
{
    long long thisTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    bool canClick = autoClickLastClickTime < thisTime - (autoClickSpeed * 10);
    
    targetX = 0;
    targetY = 0;

    if ((autoClickEnabled || murderboneEnabled) && canClick || autoMoveEnabled)
    {
        long long playerCid = 0;
        GetDefaultStatCid(&playerCid);
        playerCid >>= 32; //playerCid -= 0xCCCCCC00LL;
        if (murderboneEnabled) {
            targetTile[0] = 0;
        }
        std::vector<std::pair<int, byondtile*>> rangeList;
        std::vector<byondtile*> tiles = getTiles();
        for (size_t i = 0; i < tiles.size(); i++)
        {
            int dx = std::abs(tiles[i]->X - getPlayerX());
            int dy = std::abs(tiles[i]->Y - getPlayerY());
            if ((murderboneEnabled && *(char*)(&tiles[i]->cid) == 1 && (tiles[i]->cid >> 32) != playerCid) 
                || (!murderboneEnabled && strcmp(tiles[i]->name, targetTile) == 0)
                && WhitelistBlacklistCheck(tiles[i]->cid))
            {
                rangeList.push_back(std::pair<int, byondtile*>(dx + dy, tiles[i]));
            }
        }
        
        std::sort(rangeList.begin(), rangeList.end());
        for (std::vector<std::pair<int, byondtile*>>::iterator it = rangeList.begin(); it != rangeList.end(); ++it) {
            byondtile* tile = it->second;
            targetX = tile->X;
            targetY = tile->Y;
            int dx = std::abs(tile->X - getPlayerX());
            int dy = std::abs(tile->Y - getPlayerY());
            if (canClick && dx <= autoClickMaxRange && dy <= autoClickMaxRange && (autoClickEnabled || murderboneEnabled)) {
                ClickOn(tile->cid, tile->X, tile->Y);
                if (murderboneEnabled) {
                    strncpy_s(targetTile, 64, tile->name, 64);
                }
                autoClickLastClickTime = thisTime;
            }
            break;
        }
        
        for (size_t i = 0; i < tiles.size(); i++)
        {
            delete tiles[i];
        }
    }

    return;
}

#pragma region pathfinding
union Point {
    int as_int;
    POINTS as_point;
};
bool operator==(Point a, Point b) {
    return a.as_int == b.as_int;
}
const Point UNDEFINED_POINT = (Point)INT_MAX;

int Heuristic(Point a, Point b) {
    return (std::abs(b.as_point.x - a.as_point.x) + std::abs(b.as_point.y - a.as_point.y)) * 20;
}
std::map<int, char> passability;
std::list<std::pair<Point, int>> GetNeighbors(Point current) {
    std::list<std::pair<Point, int>> adjacent;

    Point west = current;
    west.as_point.x--;
    bool westValid = passability[west.as_int] < 1;

    Point east = current;
    east.as_point.x++;
    bool eastValid = passability[east.as_int] < 1;

    Point south = current;
    south.as_point.y--;
    bool southValid = passability[south.as_int] < 1;

    Point north = current;
    north.as_point.y++;
    bool northValid = passability[north.as_int] < 1;

    if (westValid) { // west
        adjacent.push_front({ west, 10 });
    }
    if (eastValid) { // east
        adjacent.push_front({ east, 10 });
    }
    if (southValid) { // south
        adjacent.push_front({ south, 10 });
    }
    if (northValid) { // north
        adjacent.push_front({ north, 10 });
    }

    Point southwest = south;
    southwest.as_point.x--;

    Point southeast = south;
    southeast.as_point.x++;

    Point northwest = north;
    northwest.as_point.x--;

    Point northeast = north;
    northeast.as_point.x++;

    // diagonals:
    if ((westValid || southValid) && (passability[southwest.as_int] < 1)) { // south-west
        adjacent.push_front({ southwest, 14 });
    }
    if ((eastValid || southValid) && (passability[southeast.as_int] < 1)) { // south-east
        adjacent.push_front({ southeast, 14 });
    }
    if ((westValid || northValid) && (passability[northwest.as_int] < 1)) { // north-west
        adjacent.push_front({ northwest, 14 });
    }
    if ((eastValid || northValid) && (passability[northeast.as_int] < 1)) { // north-east
        adjacent.push_front({ northeast, 14 });
    }
    return adjacent;
}
struct Score {
    unsigned int fScore;
    unsigned int gScore;
};
bool Pathfind(Point startPoint, Point target, Point* result) {
    std::unordered_set<int> open;
    std::unordered_map<int, Score> score;
    std::unordered_map<int, int> cameFrom;

    //Center
    open.insert(startPoint.as_int);
    Score startScore = Score();
    startScore.gScore = 0;
    startScore.fScore = Heuristic(startPoint, target);
    score[startPoint.as_int] = startScore;

    int tries = 0;
    Point closest = UNDEFINED_POINT;
    while (open.size() > 0 && ++tries < 2000) {
        Point current = UNDEFINED_POINT;
        // Set our current node to the node with the minimum fScore in the open list
        for (std::unordered_map<int, Score>::iterator it = score.begin(); it != score.end(); ++it) {
            if ((current == UNDEFINED_POINT || it->second.fScore < score.at(current.as_int).fScore) && (open.find(it->first) != open.end())) {
                current.as_int = it->first;
            }
        }
        // There are no more open nodes, and we could not reach the target.
        if (current == UNDEFINED_POINT) {
            return false;
        }
        if (current == target) {
            closest = current;
            break;
        }
        open.erase(current.as_int);

        std::list<std::pair<Point, int>> neighbors = GetNeighbors(current);
        for (auto it = neighbors.begin(); it != neighbors.end(); ++it) {
            Point neighbor = it->first;
            int cost = it->second;

            int tentativeG = score.at(current.as_int).gScore + cost;
            int currentG = INT_MAX;
            if (score.count(neighbor.as_int)) {
                currentG = score.at(neighbor.as_int).gScore;
            }
            if (tentativeG < currentG) {
                cameFrom.insert_or_assign(neighbor.as_int, current.as_int);
                Score newScore = Score();
                newScore.fScore = tentativeG + Heuristic(neighbor, target);
                newScore.gScore = tentativeG;
                score.insert_or_assign(neighbor.as_int, newScore);
                open.insert(neighbor.as_int);
            }
        }
    }
    // We didn't reach the target. Instead, search for the closest node that we got to, according to the heuristic.
    if (closest == UNDEFINED_POINT) {
        for (std::unordered_map<int, Score>::iterator it = score.begin(); it != score.end(); ++it) {
            if (closest == UNDEFINED_POINT || it->second.fScore < score.at(closest.as_int).fScore) {
                closest.as_int = it->first;
            }
        }
    }
    if (closest != UNDEFINED_POINT) {
        Point previous = UNDEFINED_POINT;
        Point thisNode = closest;
        while (cameFrom.count(thisNode.as_int)) {
            previous = thisNode;
            thisNode.as_int = cameFrom.at(thisNode.as_int);
        }
        *result = previous;
        return true;
    }
    // Something went pretty wrong, there are no nodes to go toward
    return false;
}

std::unordered_set<std::string> unpassableNames{"wall", "grille", "reinforced wall", "lava", "rock", "soap", "table", "reinforced table", "glass table", "wooden table", "rack", "banana peel", "tank dispenser", "freezer", "heater", "disposal unit", "hydroponics tray"};
int horizInputCancelTicks = 0;
int vertInputCancelTicks = 0;
bool getPathfindTarget(Point* target) {
    if (autoMoveX == 0 && autoMoveY == 0) {
        if (targetX == 0 && targetY == 0) {
            return false;
        }
        target->as_point.x = targetX;
        target->as_point.y = targetY;
    }
    else {
        target->as_point.x = autoMoveX;
        target->as_point.y = autoMoveY;
    }
    return true;
}

void __fastcall DoPathfind()
{
    long long thisTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    bool shouldPath = pathfindLastCheck < thisTime - 100;

    if (horizInputCancelTicks != 0) {
        horizInputCancelTicks--;
    }
    if (vertInputCancelTicks != 0) {
        vertInputCancelTicks--;
    }
    //PIPELOG("h:%u v:%u", horizInputCancelTicks, vertInputCancelTicks);

    if (autoMoveEnabled && shouldPath) {
        std::vector<byondtile*> tiles = getTiles();
        Point tilePos;
        // Reset the passability for all tiles with something onscreen.
        for (size_t i = 0; i < tiles.size(); i++)
        {
            tilePos.as_point.x = tiles[i]->X;
            tilePos.as_point.y = tiles[i]->Y;
            passability[tilePos.as_int] = 0;
        }
        // Calculate new passability using everything onscreen
        for (size_t i = 0; i < tiles.size(); i++)
        {
            if (unpassableNames.contains(tiles[i]->name)) {
                tilePos.as_point.x = tiles[i]->X;
                tilePos.as_point.y = tiles[i]->Y;
                passability[tilePos.as_int]++;
            }
        }
        // Free memory
        for (size_t i = 0; i < tiles.size(); i++)
        {
            delete tiles[i];
        }

        Point start;
        start.as_point.x = getPlayerX();
        start.as_point.y = getPlayerY();

        Point target;
        if (!getPathfindTarget(&target)) {
            return;
        }

        Point result;
        if (target == start) {
            return;
        }
        else if (Pathfind(start, target, &result))
        {
            int yDiff = result.as_point.y - start.as_point.y;
            int xDiff = result.as_point.x - start.as_point.x;
            bool yAdjacent = std::abs(yDiff) <= 2;
            bool xAdjacent = std::abs(xDiff) <= 2;
            if (!yAdjacent || yAdjacent && (vertInputCancelTicks == 0)) {
                if (yAdjacent) {
                    vertInputCancelTicks = std::abs(yDiff) == 1 ? 10 : 5;
                }
                if (yDiff > 0) {
                    ParseCommand(client, client, "KeyDown \"W\"", 0, 0);
                    ParseCommand(client, client, "KeyUp \"W\"", 0, 0);
                }
                else if (yDiff < 0) {
                    ParseCommand(client, client, "KeyDown \"S\"", 0, 0);
                    ParseCommand(client, client, "KeyUp \"S\"", 0, 0);
                }
            }
            if (!xAdjacent || xAdjacent && (horizInputCancelTicks == 0)) {
                if (xAdjacent) {
                    horizInputCancelTicks = std::abs(xDiff) == 1 ? 10 : 5;
                }
                if (xDiff > 0) {
                    ParseCommand(client, client, "KeyDown \"D\"", 0, 0);
                    ParseCommand(client, client, "KeyUp \"D\"", 0, 0);
                }
                else if (xDiff < 0) {
                    ParseCommand(client, client, "KeyDown \"A\"", 0, 0);
                    ParseCommand(client, client, "KeyUp \"A\"", 0, 0);
                }
            }
        }
    }

    return;
}
#pragma endregion

#pragma region hooks
void __cdecl Tick_hook()
{
    FireConnection("Tick");

    DoAutoClick();
    DoPathfind();
    long long thisTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    if (autoFlipper) {
        if (lastFlipTime < thisTime - 100) {
            lastFlipTime = thisTime;
            ParseCommand(client, client, "Say \"*flip\"", 0, 1);
        }
    }
    if (autoScream) {
        if (lastScreamTime < thisTime - 10500) {
            lastScreamTime = thisTime;
            ParseCommand(client, client, "Say \"*scream\"", 0, 1);
        }
    }
    if (autoLaugh) {
        if (lastLaughTime < thisTime - 500) {
            lastLaughTime = thisTime;
            ParseCommand(client, client, "Say \"*laugh\"", 0, 1);
        }
    }
    if (resetViewPixelOffset) {
        *viewPixelOffset = 0;
    }
    /*if (higherView) {
        currentW = realW + extraW;
        currentH = realH + extraH;
    } else {
        if (*viewW1 != currentW || *viewH1 != currentH) {
            realW = *viewW1;
            realH = *viewH1;
        }
        currentW = realW;
        currentH = realH;
    }
    *viewW1 = currentW;
    *viewW2 = currentW;
    *viewH1 = currentH;
    *viewH2 = currentH;*/
    // failure
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 288) = 25;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 290) = 23;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 300) = 25;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 304) = 23;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 308) = 25;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 312) = 23;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 332) = 25;
    //*(char*)(*(char*)(dllBase + 0x003888d0) + 336) = 23;
    return Tick();
}

int __fastcall IsByondMember_hook(void*, void*, int *num)
{
    return TRUE;
}

int __fastcall GenMouseMoveCommand_hook(void* pThis, void* unused, MouseParams* params)
{
    latestMouseCid = params->Cid;
    return GenMouseMoveCommand(pThis, unused, params);
}

int __fastcall GenClickCommand_hook(void* pThis, void* unused, MouseParams* params)
{
    if (ImGui::GetCurrentContext()) {
        ImGuiIO& imio = ImGui::GetIO();
        if (imio.WantCaptureMouse) {
            return 1;
        }
    }
    FireMouseConnection("MouseLClick", params);
    return GenClickCommand(pThis, unused, params);
}   

int __fastcall GenMouseUpCommand_hook(void* pThis, void* unused, MouseParams* params)
{
    FixMouseParams(params);
    if (ImGui::GetCurrentContext()) {
        ImGuiIO& imio = ImGui::GetIO();
        if (imio.WantCaptureMouse) {
            return 1;
        }
    }
    FireMouseConnection("MouseLUp", params);
    return GenMouseUpCommand(pThis, unused, params);
}

// hides overlays like the welding mask. does this by checking if object is on fullscreen overlay plane (20 for yogstation, 500 for tg)
// couldn't get checking based off of layer to work, just use plane
void checkAndHideOverlay(int obj) {
    //long long Cid = *(long long*)(tile + 124);
    //char* name = GetCidName(client, client, Cid);
    float layer = *(float*)(obj + 20);
    short plane = *(short*)(obj + 36);
    char subplane = *(char*)(obj + 38);

    //PIPELOG("%f %hu %hhu", layer, *(short*)(tile + 116), *(char*)(tile + 118));
    if (currentServer == codebase::Tg ? plane == 23 : currentServer == codebase::Bee ? plane == 22 : (plane == 20 || plane == 500)) {//if (strcmp(name, "") == 0) {
        *(char*)(obj + 47) = 0;
    }
#ifdef _DEBUG
    if (DEBUG_shouldHideLayer && layer == DEBUG_hideLayer || DEBUG_shouldHidePlane && plane == DEBUG_hidePlane) {
        *(char*)(obj + 47) = 0;
    }
#endif
}

std::unordered_map<std::string, char> match;
void modifyMapIcons() {
    float r, g, b;
    const int cycleTime = 2000;
    long long thisTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    ImGui::ColorConvertHSVtoRGB((float)(thisTime % cycleTime) / cycleTime, 1.0f, 1.0f, r, g, b);

    if (fullbrightEnabled || hideOverlays || hideObjs > 0 || autoMoveEnabled) {
        if (mapIconsList != 0) {
            short numPlanes = *(short*)(mapIconsList + 8);
            for (int p = 0; p < numPlanes; p++)
            {
                int planePtr = *(int*)(mapIconsList + 4) + p * 200;
                char* planeName = GetCidName(client, client, *(long long*)(planePtr + 12));
                short start = *(short*)(planePtr + 158); // start index of children of this object in the map icons list
                short end = *(short*)(planePtr + 160); // end index of children of this object in the map icons list
                if (autoMoveEnabled) {
                    Point target;
                    if (getPathfindTarget(&target)) {
                        for (int j = start; j < end; j++)
                        {
                            int obj = *(int*)(mapIconsList + 4) + j * 200;
                            short tileX = *(short*)(obj + 92);
                            short tileY = *(short*)(obj + 94);
                            if (tileX == target.as_point.x && tileY == target.as_point.y) {
                                *(unsigned char*)(obj + 44) = (unsigned char)((float)*(unsigned char*)(obj + 44) * r);
                                *(unsigned char*)(obj + 45) = (unsigned char)((float)*(unsigned char*)(obj + 45) * g);
                                *(unsigned char*)(obj + 46) = (unsigned char)((float)*(unsigned char*)(obj + 46) * b);
                            }
                            else {
                                Point pos;
                                pos.as_point.x = tileX;
                                pos.as_point.y = tileY;
                                if (passability[pos.as_int] > 0) {
                                    *(unsigned char*)(obj + 45) = 0;
                                    *(unsigned char*)(obj + 46) = 0;
                                }
                            }
                        }
                    }
                }
                if (fullbrightEnabled && (
                    strcmp(planeName, "lighting plane master") == 0 || strcmp(planeName, "lighting_plane") == 0 || strcmp(planeName, "Lighting plate #1") == 0 || strcmp(planeName, "Lighting plate #0") == 0
                    )) {
                    *(char*)(planePtr + 47) = brightnessAlpha;
                }
                if (hideOverlays) {
                    checkAndHideOverlay(planePtr);
                    for (int j = start; j < end; j++)
                    {
                        checkAndHideOverlay(*(int*)(mapIconsList + 4) + j * 200);
                    }
                }
                // 0 none apply 1 extra apply 2 floor applies
                // val 1 to hide all val 2 to hide floor val 0 if off
                if (hideObjs > 0) {
                    for (int j = start; j < end; j++)
                    {
                        int tile = *(int*)(mapIconsList + 4) + j * 200;
                        long long Cid = *(long long*)(tile + 12);
                        char* name = GetCidName(client, client, Cid);

                        std::string nameStr = std::string(name);
                        std::unordered_map<std::string, char>::iterator f = match.find(name);
                        char val;
                        if (f == match.end()) {
                            val = (nameStr == "terminal" || nameStr.find("camera") != std::string::npos || nameStr == "airtight plastic flaps" || nameStr == "light fixture" || nameStr == "potted plant" || nameStr == "navigation beacon" || nameStr == "firelock" || nameStr.find("holopad") != std::string::npos) +
                                (nameStr.find("pipe") != std::string::npos || nameStr.find("vent") != std::string::npos || nameStr.find("scrubber") != std::string::npos || nameStr == "power cable") * 2;
                            match.insert(std::pair<std::string, char>(nameStr, val));
                        }
                        else {
                            val = f->second;
                        }
                        if (val >= (1 << (hideObjs - 1))) {
                            *(char*)(tile + 47) = detailAlpha;
                        }
                    }
                }
            }
        }
    }
}

char __cdecl MapIcons_hook(char* name, int* mapIconsListPtr, char param1, char param2)
{
    char ret = MapIcons(name, mapIconsListPtr, param1, param2);

    if (param2 == 1 && mapIconsListPtr != (void*)0) {
        modifyMapIcons();
    }
    return ret;
}

char _fastcall GetCidMouseOpacity_hook(void* pThis, void* unused, long long Cid) {
    if (hideObjs != 0) {
        std::unordered_map<std::string, char>::iterator f = match.find(GetCidName(client, client, Cid));
        if (f != match.end() && f->second >= (1 << (hideObjs - 1))) {
            return 0;
        }
    }
    return GetCidMouseOpacity(pThis, unused, Cid);
}
#pragma endregion

std::string getKeybindString(std::pair<std::string, KeyBind> binding) {
    std::string result;
    if (editingBind != nullptr && *editingBind == binding.first) {
        result = std::string("Press a key");
    } else if (binding.second.disabled) {
        result = std::string("Unbound");
    } else {
        std::vector<const char*> list;
        if (binding.second.ctrl) list.push_back("Ctrl");
        if (binding.second.shift) list.push_back("Shift");
        if (binding.second.alt) list.push_back("Alt");
        if (binding.second.isClick) {
            int key = binding.second.key;
            list.push_back(key == 0 ? "LMouse" : key == 1 ? "RMouse" : key == 2 ? "MMouse" : key == 3 ? "Mouse4" : key == 4 ? "Mouse5" : "MouseUnknown");
        }
        else {
            CHAR name[1024];
            UINT scanCode = MapVirtualKeyW(binding.second.key, MAPVK_VK_TO_VSC);
            int result = GetKeyNameTextA(scanCode << 16, name, 1024);
            if (result > 0)
            {
                list.push_back(name);
            }
            else {
                list.push_back("Unknown");
            }
        }
        auto first = list.begin();
        for (auto it = first; it != list.end(); ++it) {
            if (it == first) {
                result = std::string(*it);
            }
            else {
                result += " + ";
                result += *it;
            }
        }
    }
    return result + "##" + binding.first;
}

void SetAimbotTarget() {
    if (latestMouseCid != 0 && (*(char*)(&latestMouseCid) == 1 || !targetMobs)) {
        char* name = GetCidName(client, client, latestMouseCid);
        if (name != NULL) {
            PIPELOG("Selected %s", name);
            strncpy_s(targetTile, 64, name, 64);
        }
    }
}

void ToggleAimbot() {
    autoClickEnabled = !autoClickEnabled;
}

void RenderImGui()
{
    ImGui::Begin("ByondControl", 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysAutoResize);
    ImGui::PushItemWidth(200.0f);
    if (ImGui::TreeNode("Help")) {
        ImGui::Text("Coordinates are 1 less than GPS");
        ImGui::Text("and in-game coordinates in both X and Y");
        ImGui::Text("Move To Target: Naive pathfinding for tg");
        ImGui::Text("While on higher view range, byond will");
        ImGui::Text("not display objects far away.");
        ImGui::Text("Hide overlays is to hide tg welding");
        ImGui::Text("overlays but is in beta and is poor.");
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Keybinds")) {
        //ImGui::Checkbox("Eat input", &eatInput);
        for (auto it = bindsList.begin(); it != bindsList.end(); ++it) {
            ImGui::Text(it->first.c_str());
            if (ImGui::Button(getKeybindString(*it).c_str())) {
                if (editingBind != nullptr && *editingBind == it->first) {
                    it->second.disabled = true;
                    editingBind == nullptr;
                }
                else {
                    editingBind = &(it->first);
                }
            }
        }
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Aimbot")) {
        ImGui::Checkbox("Keybind Only Targets Mobs", &targetMobs);
        ImGui::Checkbox("Murderbone", &murderboneEnabled);
        if (!murderboneEnabled) {
            ImGui::InputText("Target", targetTile, IM_ARRAYSIZE(targetTile));
            ImGui::Checkbox("Autoclick Targets", &autoClickEnabled);
            //ImGui::Checkbox("Aimbot by Name", &murderboneEnabled);
        }
        ImGui::SliderInt("Autoclick Range", &autoClickMaxRange, 0, 20);
        ImGui::SliderInt("Autoclick Speed (cs)", &autoClickSpeed, 1, 200);
        ImGui::Checkbox("Move To Target", &autoMoveEnabled);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Settings")) {
        int selectedServer = (int)currentServer;
        ImGui::RadioButton("/tg/station (default)", &selectedServer, (int)codebase::Tg);
        ImGui::SameLine();
        ImGui::RadioButton("Beestation", &selectedServer, (int)codebase::Bee);
        ImGui::SameLine();
        ImGui::RadioButton("Yogstation", &selectedServer, (int)codebase::Yog);
        currentServer = static_cast<codebase>(selectedServer);

        static int selectedSightButton = 0;
        static int prevSelectedSightButton = 0;
        ImGui::RadioButton("Default", &selectedSightButton, 0);
        ImGui::SameLine();
        ImGui::RadioButton("XRay", &selectedSightButton, 1);
        ImGui::SameLine();
        ImGui::RadioButton("XRay+", &selectedSightButton, 2);
        if (selectedSightButton != prevSelectedSightButton) {
            prevSelectedSightButton = selectedSightButton;

            int current = *sight;
            switch (selectedSightButton) {
            case 0:
                *sight = 0;
                break;
            case 1:
                *sight = 1916;
                break;
            case 2:
                *sight = 1918;
                break;
            }
        }
        
        /*ImGui::Checkbox("Modify View Range", &higherView);
        if (higherView) {
            ImGui::Indent();
            ImGui::SliderInt("+Width", &extraW, 1, 10);
            ImGui::SliderInt("+Height", &extraH, 1, 10);
            ImGui::Unindent();
        }*/
        if (ImGui::Button("Load Script")) {
            LoadLuaFile();
        }
        if (ImGui::Button("Clear Script Connections")) {
            ClearConnections();
            ClearLuaKeyBindings();
        }
        ImGui::Checkbox("Lua Windows", &showLuaWindows);
        ImGui::Checkbox("Lua Output", &luaConsoleEnabled);
        ImGui::SameLine();
        if (ImGui::Button("Clear##luaoutput")) {
            ClearLuaConsole();
        }
        ImGui::Checkbox("Brightness", &fullbrightEnabled);
        if (fullbrightEnabled) {
            ImGui::Indent();
            ImGui::SliderInt("Alpha", &brightnessAlpha, 0, 254);
            ImGui::Unindent();
        }
        ImGui::Checkbox("Hide Overlays", &hideOverlays);
        if (ImGui::Button("Hide Detail Objects")) {
            hideObjs = (hideObjs + 1) % 3;
        }
        if (hideObjs != 0) {
            ImGui::Indent();
            ImGui::SliderInt("Detail Alpha", &detailAlpha, 0, 254);
            ImGui::Unindent();
        }
        //ImGui::Checkbox("Reset view pixel offset", &resetViewPixelOffset);
        ImGui::Checkbox("Show Objects List", &objectsList);
        ImGui::TreePop();
    }
    if (ImGui::TreeNode("Fun")) {
        ImGui::Checkbox("Auto *flip", &autoFlipper);
        ImGui::Checkbox("Auto *scream", &autoScream);
        ImGui::Checkbox("Auto *laugh", &autoLaugh);
        ImGui::TreePop();
    }
    #ifdef _DEBUG
    if (ImGui::TreeNode("Debug")) {
        if (ImGui::Button("Print plane names")) {

            if (mapIconsList != 0) {
                short numPlanes = *(short*)(mapIconsList + 8);
                for (int p = 0; p < numPlanes; p++)
                {
                    int planePtr = *(int*)(mapIconsList + 4) + p * 200;
                    char* planeName = GetCidName(client, client, *(long long*)(planePtr + 12));
                    short start = *(short*)(planePtr + 162); // start index of children of this object in the map icons list
                    short end = *(short*)(planePtr + 164) - 1; // end index of children of this object in the map icons list
                    PIPELOG("%d: %s", p, planeName);
                }
            }
        }
        ImGui::Checkbox("Hide layer", &DEBUG_shouldHideLayer);
        if (DEBUG_shouldHideLayer) {
            ImGui::SliderInt("Layer #", &DEBUG_hideLayer, 0, 255);
        }
        ImGui::Checkbox("Hide plane", &DEBUG_shouldHidePlane);
        if (DEBUG_shouldHidePlane) {
            ImGui::SliderInt("Plane #", &DEBUG_hidePlane, 0, 255);
        }

        static bool DEBUG_pathfindShowPassability = false;
        ImGui::Checkbox("Pathfind", &DEBUG_pathfindShowPassability);
        if (DEBUG_pathfindShowPassability) {
            for (int y = 5; y >= -5; y--) {
                ImGui::Text("");
                for (int x = -5; x <= 5; x++) {
                    Point base;
                    base.as_point.x = getPlayerX() + x;
                    base.as_point.y = getPlayerY() + y;
                    ImGui::SameLine();
                    if (x == 0 && y == 0) {
                        ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(200, 200, 200, 255));
                    }
                    ImGui::Text("%1hhX", passability.contains(base.as_int) ? passability[base.as_int] : 0);
                    if (x == 0 && y == 0) {
                        ImGui::PopStyleColor();
                    }
                }
            }
        }

        ImGui::TreePop();
    }
    #endif // DEBUG

    ImGui::Text("X:%i Y:%i", getPlayerX(), getPlayerY());
    ImGui::End();

    // Second window
    if (objectsList) {
        ImGui::Begin("Object List");
        if (ImGui::TreeNode("Help")) {
            ImGui::Text("Click on a name to set them as aimbot target");
            ImGui::Text("for when the murderbone mode is not enabled.");
            ImGui::Text("Click the black box next to a name for blacklist toggle.");
            ImGui::Text("Click the white box next to a name for whitelist toggle.");
            ImGui::Text("Blacklisted mobs will not be targeted and this takes precedent over whitelist.");
            ImGui::Text("Whitelisted mobs will be the only ones targeted while whitelist is enabled.");
            ImGui::Text("Basically: Check the BLACK box next to your friends' names. :)");
            ImGui::Text("Or, enable the whitelist and check the white box next to the names of sec officers.");
            ImGui::TreePop();
        }
        ImGui::Checkbox("Whitelist", &mobWhitelistEnabled);
        ImGui::SameLine();
        if (ImGui::Button("Clear##objects")) {
            whitelist.clear();
            blacklist.clear();
        }
        std::vector<byondtile*> tiles = getTiles();
        std::unordered_set<long long> alreadyDisplayed;
        for (size_t i = 0; i < tiles.size(); ++i)
        {
            // second condition checks that it's a mob or at least part of a mob
            if (alreadyDisplayed.count(tiles[i]->cid) == 0 && *(char*)(tiles[i]->dataPtr + 12) == 1) {
                alreadyDisplayed.insert(tiles[i]->cid);
                std::unordered_set<long long>::iterator whitelistIt = whitelist.find(tiles[i]->cid);
                bool whitelistStateOg = whitelistIt != whitelist.end();
                bool whitelistState = whitelistStateOg;

                std::unordered_set<long long>::iterator blacklistIt = blacklist.find(tiles[i]->cid);
                bool blacklistStateOg = blacklistIt != blacklist.end();
                bool blacklistState = blacklistStateOg;

                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(200, 200, 200, 255));
                ImGui::Checkbox((std::string("##w") + std::to_string(tiles[i]->cid)).c_str(), &whitelistState);
                if (whitelistState && !whitelistStateOg) {
                    whitelist.insert(tiles[i]->cid);
                }
                else if (!whitelistState && whitelistStateOg) {
                    whitelist.erase(whitelistIt);
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();

                ImGui::PushStyleColor(ImGuiCol_FrameBg, IM_COL32(50, 50, 50, 255));
                ImGui::Checkbox((std::string("##b") + std::to_string(tiles[i]->cid)).c_str(), &blacklistState);
                if (blacklistState && !blacklistStateOg) {
                    blacklist.insert(tiles[i]->cid);
                }
                else if (!blacklistState && blacklistStateOg) {
                    blacklist.erase(blacklistIt);
                }
                ImGui::PopStyleColor();

                ImGui::SameLine();

                // troubleshoot X&Y
                //ImGui::Text("X %hd Y %hd", tiles[i]->X, tiles[i]->Y);
                //ImGui::SameLine();

                if (ImGui::Button(tiles[i]->name)) {
                    strncpy_s(targetTile, 64, tiles[i]->name, 64);
                }
            }
            delete tiles[i];
        }
        ImGui::End();
    }

    /* ImGui::Begin("Object List");
    short numPlanes = *(short*)(mapIconsList + 8) - 1;
    ImGui::Text("%hd", numPlanes);
    for (int p = 0; p < numPlanes; p++)
    {
        int planePtr = *(int*)(mapIconsList + 4) + p * 200;
        ImGui::Text("Name: %s", GetCidName(client, client, *(long long*)(planePtr + 12)));
        /*for (int o = 0; o < 16; o++) {
            try {
                ImGui::Text("%hd %hd %hd %hd", *(short*)(planePtr + o * 4), *(short*)(planePtr + o * 4 + 2));
            }
            catch (const std::exception &e) {

            }
        }
        // 110
    }
    ImGui::End();*/

    LuaConsole();

    if (showLuaWindows) {
        LuaWindows();
    }
}

void InitKeybinds() {
    struct KeyBind selectTargBind;
    selectTargBind.disabled = false;
    selectTargBind.ctrl = false;
    selectTargBind.shift = true;
    selectTargBind.alt = true;
    selectTargBind.isClick = true;
    selectTargBind.key = 0;
    selectTargBind.func = (int)&SetAimbotTarget;
    selectTargBind.isLuaBinding = false;
    bindsList.emplace("Select Target", selectTargBind);

    struct KeyBind toggleAutoclickBind;
    toggleAutoclickBind.disabled = false;
    toggleAutoclickBind.ctrl = true;
    toggleAutoclickBind.shift = true;
    toggleAutoclickBind.alt = true;
    toggleAutoclickBind.isClick = true;
    toggleAutoclickBind.key = 0;
    toggleAutoclickBind.func = (int)&ToggleAimbot;
    toggleAutoclickBind.isLuaBinding = false;
    bindsList.emplace("Toggle Autoclicking", toggleAutoclickBind);
}

void RunJavascript(char* script) {
    int result = 0;
    void* nullString = DMString(&result, 0, "(null)");
    for (auto it = browsers.begin(); it != browsers.end(); ++it) {
        //PIPELOG("%s", script);
        InvokeScript(*it, 0, script, (char*)nullString);
    }
}

int __fastcall InvokeScript_hook(void* pThis, void* unused, char* s1, char* s2) {
    int result = InvokeScript(pThis, unused, s1, s2);
    //PIPELOG("hruhu. %x %x %s %s %u", pThis, unused, s1, s2, result);
    FireScriptConnection(s1);
    return result;
}

char __fastcall ParseCommand_hook(void* pThis, void* unused, const char* s1, char c1, char c2) {
    char result = ParseCommand(pThis, unused, s1, c1, c2);
    //PIPELOG("parse. %x %x %s %hhu %hhu %hhu", pThis, unused, s1, c1, c2, result);
    //FireCommandConnection(s1);
    FireCommandConnection(s1, c1);
    return result;
}

void* __fastcall htmlCtrl_hook(void* pThis, void* unused, void* arg) {
    browsers.push_back(pThis);
    void* result = htmlCtrl(pThis, unused, arg);
    //PIPELOG("waaaaaah %x %x", pThis, unused);
    return result;
}

void __fastcall htmlCtrlDestructor_hook(void* pThis, void* unused) {
    //PIPELOG("wah brwoser destroyed");
    for (auto it = browsers.begin(); it != browsers.end(); ++it) {
        browsers.erase(it);
        break;
    }
    htmlCtrlDestructor(pThis, unused);
}

/*void __fastcall mapWindowTick_hook(void* pThis, void* unused, char c1) {
    mapWindowTick(pThis, unused, c1);
    //PIPELOG("hruhu. %x %x %hhx", pThis, unused, c1);
    if (c1 == 1 && mainWindow == nullptr) {
        mainWindow = pThis;
        PIPELOG("mainwindow: %x %x", mainWindow, *(int*)pThis - base);
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)UIThread, NULL, 0, NULL);
    }
}*/

void* __fastcall clientMapCtrl_hook(void* pThis, void* unused, int a, void* b, void* c) {
    void* result = clientMapCtrl(pThis, unused, a, b, c);
    //PIPELOG("hruhu. %x %x %hhx", pThis, unused, c1);
    //if (c1 == 1 && mainWindow == nullptr) {
    if (a == 0x3e8) {
        mainWindow = pThis;
        //PIPELOG("mapview: %x %x %x %x", pThis, a, b, c);
        int windowCtrl = *(int*)((int)pThis + 0xcc);
        //PIPELOG("wind: %x container: %x", *(int*)(windowCtrl + 0x1c), *(int*)(windowCtrl + 0x20));
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)UIThread, NULL, 0, NULL);
    }
    return result;
}

/*void __fastcall D3D_hook(void* pThis, void* unused, char a) {
    makeD3D(pThis, unused, a);
    PIPELOG("%x %hhd", pThis, a);
    UIThread();
}*/

/*
void __fastcall testtest_hook(void* pThis, void* unused, int a, int b, void* c) {
    testtest(pThis, unused, a, b, c);
    PIPELOG(":O :O HAI %x %x %x %x", pThis, a, b, c);
}

int __fastcall sight_hook(int a) {
    int result = sight(a);
    //PIPELOG(":3 %x %x", a, result);
    return result;
}
*/

/*char __fastcall obscure_hook(void* pThis, void* unused, short a, short b, short c, short d, short e, short f, short g, char h, char i, char j, char k, char l) {
    char result = obscure(pThis, unused, a, b, c, d, e, f, g, h, i, j, k, l);
    //PIPELOG("%hhd", *(char*)((int)pThis + 52));
    PIPELOG("%hd %hd %hd %hd %hd %hd %hd %hhd %hhd %hhd %hhd %hhd", a, b, c, d, e, f, g, h, i, j, k, l);
    //char result = 1;
    PIPELOG("o %hhd %hhd %hhd %hhd %hhd %hhd", *(char*)((int)pThis + 46), *(char*)((int)pThis + 47), *(char*)((int)pThis + 48), *(char*)((int)pThis + 49), *(char*)((int)pThis + 50), *(char*)((int)pThis + 52));
    //*(char*)((int)pThis + 50) = 1;
    return result;
}

int __cdecl obscure2_hook(int a) {
    int result = obscure2(a);
    PIPELOG("%X %X", a, result);
    return result;
}

int __fastcall networkmsg_hook(void* pThis, void* unused, short* a, int* pointer) {
    int result = networkmsg(pThis, unused, a, pointer);
    PIPELOG("%X %hX %X", pThis, *a, *pointer);
    return result;
}

int __cdecl viewmsg_hook(int a) {
    int result = viewmsg(a);
    //PIPELOG("%X %X", a, result);
    PIPELOG("%X %hX %hX %hX %hX %hX %hX %hX %hX", *(int*)(a + 4), *(short*)(*(int*)(a + 12) + 8), *(short*)(*(int*)(a + 12) + 10), *(int*)(a + 4), *(short*)(*(int*)(a + 12) + 12), *(short*)(*(int*)(a + 12) + 14), *(short*)(*(int*)(a + 12) + 16), *(short*)(*(int*)(a + 12) + 18), *(short*)(*(int*)(a + 12) + 20), *(short*)(*(int*)(a + 12) + 22));
    *(short*)(*(int*)(a + 12) + 8) = 0x04;
    *(short*)(*(int*)(a + 12) + 10) = 0x04;
    *(short*)(*(int*)(a + 12) + 12) = 0x04;
    *(short*)(*(int*)(a + 12) + 14) = 0x04;
    return result;
}*/

DWORD WINAPI Main(HMODULE hModule)
{
    PIPELOG("talkey talkey");

    // get BC directory
    try {
        DWORD dataSize{};
        if (RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\ByondControl", "installdir", RRF_RT_REG_SZ, NULL, NULL, &dataSize) != ERROR_SUCCESS) {
            throw 1;
        }

        bcpath.resize(dataSize / sizeof(char));

        if (RegGetValueA(HKEY_CURRENT_USER, "SOFTWARE\\ByondControl", "installdir", RRF_RT_REG_SZ, NULL, &bcpath[0], &dataSize) != ERROR_SUCCESS) {
            throw 1;
        }

        DWORD stringLengthInChars = dataSize / sizeof(char);
        stringLengthInChars--; // Exclude the NUL written by the Win32 API
        bcpath.resize(stringLengthInChars);
    }
    catch (int n) {
        PIPELOG("failed to get bc directory");
    }

    HKEY hKey;
    if (RegOpenKeyExA(HKEY_CURRENT_USER, "SOFTWARE\\Dantom\\BYOND", NULL, KEY_ALL_ACCESS, &hKey) == ERROR_SUCCESS) {
        DWORD buffer;
        DWORD dwSize = sizeof(buffer);
        DWORD regType = REG_DWORD;
        if (RegQueryValueExA(hKey, "bcsetting", NULL, (LPDWORD)&regType, (LPBYTE)&buffer, &dwSize) == ERROR_SUCCESS) {
            settingsVal = buffer;
        }
        RegCloseKey(hKey);
    }
    else {
        PIPELOG("failed to open registry\n");
        ExitProcess(-1);
    }

    StartCidSpoofer();
#ifdef KEYSYSTEM_ENABLED
    char key[128];
    char* keyp = &key[0];
    if (GetSavedKey(keyp)) {
        if (CheckKey(keyp) == KEYSYSTEM_RES_VALID) {
            goto start;
        }
    }
    MessageBoxA(0, "auth failed somehow", "error", MB_OK);

    return TRUE;
start:
#endif
    if (GetByondVersion(NULL) != 515 || GetByondBuild(NULL) != 1597) {
        PIPELOG("byond version needs to be 515.1597");
        return TRUE;
    }

    PIPELOG("hooking\n");

    DetourTransactionBegin();
    DetourUpdateThread(GetCurrentThread());
    DetourAttach(&(PVOID&)IsByondMember, IsByondMember_hook);
    DetourAttach(&(PVOID&)Tick, Tick_hook);
    DetourAttach(&(PVOID&)MapIcons, MapIcons_hook);
    DetourAttach(&(PVOID&)GetCidMouseOpacity, GetCidMouseOpacity_hook);
    //DetourAttach(&(PVOID&)testtest, testtest_hook);
    //DetourAttach(&(PVOID&)sight, sight_hook);
    //DetourAttach(&(PVOID&)obscure, obscure_hook);
    //DetourAttach(&(PVOID&)obscure2, obscure2_hook);
    //DetourAttach(&(PVOID&)networkmsg, networkmsg_hook);
    //DetourAttach(&(PVOID&)viewmsg, viewmsg_hook);
    DetourAttach(&(PVOID&)GenMouseMoveCommand, GenMouseMoveCommand_hook);
    DetourAttach(&(PVOID&)GenMouseUpCommand, GenMouseUpCommand_hook);
    DetourAttach(&(PVOID&)GenClickCommand, GenClickCommand_hook);
    DetourAttach(&(PVOID&)InvokeScript, InvokeScript_hook);
    DetourAttach(&(PVOID&)ParseCommand, ParseCommand_hook);
    DetourAttach(&(PVOID&)htmlCtrl, htmlCtrl_hook);
    DetourAttach(&(PVOID&)htmlCtrlDestructor, htmlCtrlDestructor_hook);
    if (ShouldMakeUI()) {
        //DetourAttach(&(PVOID&)mapWindowTick, mapWindowTick_hook);
        DetourAttach(&(PVOID&)clientMapCtrl, clientMapCtrl_hook);
        //DetourAttach(&(PVOID&)D3D, D3D_hook);
    }
    DetourTransactionCommit();

    InitKeybinds();
    LuaInit();

    //CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)PipeHandler, NULL, 0, NULL);
    return TRUE;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)Main, NULL, 0, NULL);
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

