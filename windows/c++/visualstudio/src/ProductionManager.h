#pragma once


#include <BWAPI.h>
#include <queue>

class ProductionManager
{
	std::deque<BWAPI::UnitType> m_build_queue;
	std::map<int, BWAPI::UnitType> m_try_built_or_trained;
	std::map<int, BWAPI::UnitType> m_build_order;
public:

	ProductionManager();
	void tryBuildOrTrainUnit();
	bool addToBuildQueue(const BWAPI::UnitType& unit_type);
	void addToBuildQueueFromBuildOrder();
	void reQueue(int frame_delay);
	void onFrame();
	void onUnitDestroy(BWAPI::Unit unit);
	void onUnitComplete(BWAPI::Unit unit);
	void buildGateway();
	void buildAttackUnits();
	void buildAdditionalSupply();
	bool trainUnit(const BWAPI::UnitType& unit);
};
