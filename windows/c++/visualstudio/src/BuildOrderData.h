#pragma once

#include "BWAPI.h"
#include "Global.h"
#include "StrategyManager.h"

using namespace MiraBot;

class BuildOrderData
{
public:
	void initStarterBuildOrder();
	void initProtossVTerranBuildOrder();
	void initBuildOrders();
	BuildOrderData();
	static inline std::map<int, BWAPI::UnitType> clean_build_order;
	static inline std::map<int, BWAPI::UnitType> starter_build_order;
	static inline std::map<int, BWAPI::UnitType> protoss_v_terran;
};
