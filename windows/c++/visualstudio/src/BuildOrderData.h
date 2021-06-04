#pragma once

#include "BWAPI.h"
#include "Global.h"


namespace MiraBot
{
	class BuildOrderData
	{
	public:
		void initStarterBuildOrder();
		void initProtossVTerranBuildOrder();
		void initBuildOrders();
		BuildOrderData();
		std::map<int, BWAPI::UnitType> clean_build_order;
		std::map<int, BWAPI::UnitType> starter_build_order;
		std::map<int, BWAPI::UnitType> protoss_v_terran;
	};
}
