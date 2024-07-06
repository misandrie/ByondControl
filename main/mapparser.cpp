#include "mapparser.h"
#include "curl/curl.h"

extern "C" {
#include "lua/include/lua.h"
#include "lua/include/lauxlib.h"
#include "lua/include/lualib.h"
}

#ifdef _WIN32
#pragma comment(lib, "lua/lua54.lib")
#endif

#include <string>
#include <fstream>
#include <sstream>
#include <map>
#include <regex>

using namespace std;

const regex pattern1("\"(...)\" = \\(([\\s\\S]*?)\\)");
const regex pattern2("\\((\\d+),\\d,\\d\\) = \\{\"([\\s\\S]*?)\"\\}");
const regex pattern3("\\S\\S\\S");

namespace dmm
{
	MapParser::MapParser(string buffer)
	{
		mapBuffer = buffer;

		// regex search for pointers
		string::const_iterator pos(mapBuffer.cbegin());
		smatch m;
		while (regex_search(pos, mapBuffer.cend(), m, pattern1)) {
			pointers[m[1].str()] = m[2].str();
			pos = m.suffix().first;
		}

		// do not reset iterator; its already where we need it.
		// now regex search for map layout
		while (regex_search(pos, mapBuffer.cend(), m, pattern2)) {
			int x = stoi(m[1].str());
			int y = 0;
			string sub(m[2].str().c_str());
			string::const_iterator subpos(sub.cbegin());
			smatch subm;

			while (regex_search(subpos, sub.cend(), subm, pattern3)) {
				string key = subm[0].str();
				layout[x][y] = pointers[key];
				y++;
				subpos = subm.suffix().first;
			}

			pos = m.suffix().first;
		}
	}

	string MapParser::GetDataAtCoords(int x, int y, Origin origin)
	{
		if (origin == Origin::BOTTOM_LEFT)
			return layout[x][255 - y];

		return layout[x][y];
	}

	map<int, int> MapParser::FindObjects(string query, Origin origin)
	{
		map<int, int> found;

		map<int, map<int, string>>::iterator itr;//outer
		map<int, string>::iterator ptr;//inner

		for (itr = layout.begin(); itr != layout.end(); itr++) {
			for (ptr = itr->second.begin(); ptr != itr->second.end(); ptr++) {
				int x = itr->first;
				int y = ptr->first;

				// adjust y to origin
				if (origin == Origin::BOTTOM_LEFT)
					y = 255 - y;

				string value = ptr->second;
				if (value.find(query) != string::npos)
					found[x] = y;
			}
		}

		return found;
	}
}

// lua

int MapParser_Destructor(lua_State* L)
{
	auto Obj = static_cast<std::shared_ptr<dmm::MapParser>*>(lua_touserdata(L, 1));
	Obj->reset();
	return 0;
}


int MapParser_GetDataAtCoords(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TNUMBER);
	luaL_checktype(L, 3, LUA_TNUMBER);

	int x = lua_tointeger(L, 2);
	int y = lua_tointeger(L, 3);

	auto Obj = static_cast<std::shared_ptr<dmm::MapParser>*>(lua_touserdata(L, 1));
	string data = Obj->get()->GetDataAtCoords(x, y);

	lua_pushstring(L, data.c_str());

	return 1;
}

int MapParser_FindObjects(lua_State* L)
{
	luaL_checktype(L, 1, LUA_TUSERDATA);
	luaL_checktype(L, 2, LUA_TSTRING);

	string query = lua_tostring(L, 2);

	auto Obj = static_cast<std::shared_ptr<dmm::MapParser>*>(lua_touserdata(L, 1));
	map<int, int> objects = Obj->get()->FindObjects(query);

	// {{x,y},{x,y},{x,y}}

	lua_newtable(L);

	int i = 1;
	for (auto const& o : objects) {
		lua_pushinteger(L, i);
		lua_newtable(L);

		lua_pushinteger(L, 1);
		lua_pushinteger(L, o.first);
		lua_settable(L, -3);

		lua_pushinteger(L, 2);
		lua_pushinteger(L, o.second);
		lua_settable(L, -3);

		lua_settable(L, -3);
		i++;
	}

	return 1;
}

int lua_CreateMapParser(lua_State* L)
{
	string mapBuffer = lua_tostring(L, 1);

	size_t objectSize = sizeof(std::shared_ptr<dmm::MapParser>);
	void* memory = lua_newuserdata(L, objectSize);

	auto resource = std::make_shared<dmm::MapParser>(mapBuffer);
	new(memory) std::shared_ptr<dmm::MapParser>(resource);

	luaL_getmetatable(L, "MapParser_Methods");
	lua_setmetatable(L, -2);

	return 1;
}

void LuaRegisterMapParser(lua_State* L)
{
	luaL_newmetatable(L, "MapParser_Methods");

	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	lua_pushcfunction(L, MapParser_Destructor);
	lua_setfield(L, -2, "__gc");

	lua_pushcfunction(L, MapParser_GetDataAtCoords);
	lua_setfield(L, -2, "GetDataAtCoords");

	lua_pushcfunction(L, MapParser_FindObjects);
	lua_setfield(L, -2, "FindObjects");

	lua_pop(L, 1); // pop metatable

	lua_register(L, "CreateMapParser", lua_CreateMapParser);
}