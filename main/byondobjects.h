#pragma once
#include <stdio.h>
#include <windows.h>
#include <vector>
#include "pipes.h"

class byondtile
{
public:
	byondtile(int ptr);
	long long cid;
	int dataPtr;
	char* name;
	short X, Y;
	char* planeName;
	short plane;
};

byondtile::byondtile(int ptr)
{
	dataPtr = ptr;
	cid = *(long long*)(ptr + 12);
	name = GetCidName(client, client, cid);
	X = *(short*)(ptr + 92);
	Y = *(short*)(ptr + 94);
	planeName = (char*)0;
	plane = *(short*)(ptr + 36);
}

std::vector<byondtile*> getPlanes()
{
	short numPlanes = *(short*)(mapIconsList + 8);
	std::vector<byondtile*> planes;
	for (int p = 0; p < numPlanes; p++)
	{
		int planePtr = *(int*)mapIconsList + p * 200;
		byondtile* plane = new byondtile(planePtr);
		planes.push_back(plane);
	}
	return planes;
}

std::vector<byondtile*> getTiles()
{
	short numPlanes = *(short*)(mapIconsList + 8);
	std::vector<byondtile*> tiles;
	for (int p = 0; p < numPlanes; p++)
	{
		int planePtr = *(int*)(mapIconsList + 4) + p * 200;
		char* planeName = GetCidName(client, client, *(long long*)(planePtr + 12));
		short start = *(short*)(planePtr + 158);
		short end = *(short*)(planePtr + 160);
		for (int j = start; j < end; j++)
		{
			byondtile *tile = new byondtile(*(int*)(mapIconsList + 4) + j * 200);
			tile->planeName = planeName;
			tiles.push_back(tile);
		}
	}
	return tiles;
}

short getPlayerX()
{
	return *(short*)(dllBase + 0x00386f38);
}

short getPlayerY()
{
	return *(short*)(dllBase + 0x00386f3c);
}