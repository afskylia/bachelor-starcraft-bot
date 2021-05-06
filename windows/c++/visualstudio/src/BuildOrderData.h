#pragma once

#include "BWAPI.h"
/**
 * Class for storing a build order
 * if nothing else is noted in between two Supply marks - the player should build Probes and send them to mine
 */
class BuildOrder
{
public:
	BWAPI::UnitType steps[200];

	BuildOrder()
	{
		for (auto step : steps)
		{
			step = BWAPI::UnitTypes::Protoss_Probe;
		}
	}
};

class BuildOrderData
{
public:

	BuildOrderData();
	BuildOrder SimpleBuildOrder();
};
