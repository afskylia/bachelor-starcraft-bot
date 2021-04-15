#pragma once

#include "BWAPI.h"
/**
 * struct for storing one build step in a build order
 */
struct build_step
{
	int supply_level{};
	BWAPI::UnitType unit;
};

/**
 * struct for storing a build order
 * if nothing else is noted in between two Supply marks - the player should build Probes and send them to mine
 */
struct build_order
{
	std::vector<build_step> steps;
};

class BuildOrderData
{
public:

	BuildOrderData();
	build_order SimpleBuildOrder();
};
