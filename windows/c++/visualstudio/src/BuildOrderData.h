#pragma once

#include "BWAPI.h"
#include "Global.h"
#include "StrategyManager.h"

using namespace MiraBot;

class BuildOrderData
{
public:
	BuildOrderData();
	static inline std::map<int, BWAPI::UnitType> starter_build_order;
};
