#pragma once
#include <string>
#include <map>
extern "C" {
#include "lua/include/lua.h"
}

namespace dmm
{
	enum class Origin
	{
		TOP_LEFT,
		BOTTOM_LEFT,
	};

	class MapParser
	{
	public:
		MapParser(std::string buffer);
		std::map<int, int> FindObjects(std::string query, Origin origin = Origin::BOTTOM_LEFT);
		std::string GetDataAtCoords(int x, int y, Origin origin = Origin::BOTTOM_LEFT);

	private:
		std::string mapBuffer;
		std::map<std::string, std::string> pointers;
		std::map<int, std::map<int, std::string>> layout;
	};
}

void LuaRegisterMapParser(lua_State* L);