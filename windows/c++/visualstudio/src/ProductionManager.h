#pragma once
#include "BuildOrderData.h"
#include <BWAPI.h>

class ProductionManager
{
	BuildOrderData build_order_data;
	BuildOrder build = build_order_data.SimpleBuildOrder();
public:

	ProductionManager();
	void onFrame();
	void onStart();
	void onUnitComplete(BWAPI::Unit unit);
	bool trainUnit(const BWAPI::UnitType& unit);
	void buildFromBuildOrder();
	void buildGateway();
	void buildAttackUnits();
	void buildAdditionalSupply();
};
