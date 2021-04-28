#pragma once

#include "BWAPI.h"
/**
 * Class for storing a build order
 * if nothing else is noted in between two Supply marks - the player should build Probes and send them to mine
 */
class BuildOrder
{
public:
	std::map<int, BWAPI::UnitType> steps;
	BuildOrder()
	{
		for (int i = 0; i < 200; ++i)
		{
			steps.insert(std::pair<int,BWAPI::UnitType>(i, BWAPI::UnitTypes::Protoss_Probe));
		}
	}
};

class BuildOrderData
{
public:

	BuildOrderData();
	BuildOrder SimpleBuildOrder();
};
