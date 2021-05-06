#pragma once
#include <iostream>
#include <BWAPI/Unit.h>
#include <BWAPI.h>
#include <queue>

namespace MiraBot
{
	class ProductionManager
	{
		friend class Global;

		std::deque<BWAPI::UnitType> m_build_queue;
		std::map<int, BWAPI::UnitType> m_try_built_or_trained;
		std::map<int, BWAPI::UnitType> m_build_order;
		int m_last_build_frame = 0;
	public:

		ProductionManager();
		void tryBuildOrTrainUnit();
		bool addToBuildQueue(const BWAPI::UnitType& unit_type);
		std::map<BWAPI::UnitType, int> get_map_of_all_units();
		std::map<BWAPI::UnitType, int> get_map_of_required_units();
		void compareUnitsAndBuild();
		void tryCompareUnitsAndBuild();
		void onFrame();
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
		void buildGateway();
		void buildAttackUnits();
		void buildAdditionalSupply();

		int countBuildings(bool pending = true);
		int countBuildings(BWAPI::UnitType type, bool pending = true);

		int pendingBuildingsCount();
		int pendingBuildingsCount(BWAPI::UnitType type);

		int getTotalMinerals();
		int getTotalGas();

		bool buildBuilding(BWAPI::UnitType type);

		bool trainUnit(const BWAPI::UnitType& unit);
	};
}