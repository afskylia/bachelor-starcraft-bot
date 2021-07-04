#pragma once
#include <iostream>
#include <BWAPI/Unit.h>
#include <BWAPI.h>
#include <queue>
#include "BWEM/src/bwem.h"

#include "BuildOrderData.h"
#include "BuildingPlacer.h"

namespace MiraBot
{
	class ProductionManager
	{
		friend class Global;


		// units that should be built multiple times
		std::deque<BWAPI::UnitType> m_build_queue_keep_building_;

		// The last supply level we enqueued

		// Units built/enqueued from build order (used when checking onUnitDestroy), initially contains the nexus (lvl 4)
		std::vector<BWAPI::UnitType> built_units;

		std::map<int, BWAPI::UnitType> m_try_built_or_trained_;
		/*std::map<int, BWAPI::UnitType> m_build_order;*/
		int m_last_build_frame_ = 0;

		// Push the unit type at given supply lvl to build queue
		bool pushToBuildQueue(int supply_lvl);
		bool pushToBuildQueue(BWAPI::UnitType unit_type);

		// Pop from build queue if we can start building it
		void pollBuildQueue();

		BuildOrderData m_build_order_data;

	public:
		// The build queue: contains the items we need to build asap
		std::deque<BWAPI::UnitType> m_build_queue_;
		BuildingPlacer m_building_placer_;
		std::vector<int> enqueued_levels = {4};
		int prev_supply = 4;

		ProductionManager();
		void tryBuildOrTrainUnit();
		bool addToBuildQueue(const BWAPI::UnitType& unit_type);
		std::map<BWAPI::UnitType, int> getMapOfAllUnits();
		std::map<BWAPI::UnitType, int> getMapOfRequiredUnits();
		void compareUnitsAndBuild();
		void tryCompareUnitsAndBuild();
		void activateIdleBuildings();
		void trainUnitInBuilding(BWAPI::UnitType unit_type, int units_wanted);

		void printDebugData();

		void checkIfUpgradesAreAvailable();
		void onFrame();
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
		void buildGateway();
		void buildAttackUnits();
		BWAPI::UnitType getUnitToTrain(std::map<BWAPI::UnitType, int> distribution, int minerals, int gas);
		void buildAdditionalSupply();

		void pollBuildOrder();

		int countBuildings(bool pending = true);
		int countBuildings(BWAPI::UnitType type, bool pending = true);
		int countIdleBuildings(BWAPI::UnitType type, bool pending = true);

		int pendingBuildingsCount();
		int pendingBuildingsCount(BWAPI::UnitType type);

		int getTotalMinerals(bool excludingFrontOfBuildQueue = false);
		int getTotalGas(bool excludingFrontOfBuildQueue = false);

		bool buildBuilding(BWAPI::UnitType type);
		bool buildBuilding(BWAPI::UnitType type, const BWEM::Area* area);

		bool trainUnit(const BWAPI::UnitType& unit_type);
		bool trainUnit(const BWAPI::UnitType& unit, BWAPI::Unit depot);
		const BWEM::Area* createNewExpo();
	};
}
