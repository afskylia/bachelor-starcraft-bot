#pragma once
#include <BWAPI/Unit.h>

#include "WorkerData.h"

namespace MiraBot
{
	class WorkerManager
	{
		friend class Global;

	public:
		WorkerManager();

		void updateWorkerStatus();
		void sendIdleWorkersToMinerals();
		void trainAdditionalWorkers();
		void sendScout();
		void onFrame();
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);

		void setMineralWorker(BWAPI::Unit unit);

		//BWAPI::Unit getBuilder(Building& b, bool setJobAsBuilder = true);
		//BWAPI::Unit getMoveWorker(BWAPI::Position p);
		BWAPI::Unit getClosestDepot(BWAPI::Unit worker);
		//BWAPI::Unit getGasWorker(BWAPI::Unit refinery);
		//BWAPI::Unit getClosestEnemyUnit(BWAPI::Unit worker);
		//BWAPI::Unit getClosestMineralWorkerTo(BWAPI::Unit enemyUnit);
		BWAPI::Unit getWorkerScout();
		BWAPI::Position getScoutPosition(BWAPI::Unit scout);
		BWAPI::Unit getBuilder(BWAPI::UnitType type, BWAPI::Position pos);
		WorkerData getWorkerData();

		bool buildBuilding(BWAPI::UnitType type);
		void test()
		{
			std::cout << "WorkerManager!\n";
		}
	private:
		WorkerData m_workerData;
	};
}