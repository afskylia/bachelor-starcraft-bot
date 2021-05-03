#pragma once
#include "BuildOrderData.h"
#include <BWAPI.h>

class ProductionManager
{
	BuildOrderData build_order_data;
	BuildOrder build = build_order_data.SimpleBuildOrder();
	std::vector<BWAPI::UnitType> lastUnit = {};
	std::map<int, BWAPI::UnitType>::iterator it;
public:

	ProductionManager();
	void onFrame();
	void onStart();
	void onUnitComplete(BWAPI::Unit unit);
	bool trainUnit(const BWAPI::UnitType& unit);
	void buildFromBuildOrder(BWAPI::UnitType unitCompleted);
	void buildGateway();
	void buildAttackUnits();
	void buildAdditionalSupply();
	void removeFromBuildOrder(const BWAPI::UnitType& unit);
	void onUnitCreate(BWAPI::Unit unit);
};
