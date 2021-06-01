#pragma once
#include <BWAPI/Unit.h>

#include "WorkerData.h"

namespace MiraBot
{
	class WorkerManager
	{
		friend class Global;

	public:
		WorkerData m_workerData;
		WorkerManager();

		void updateWorkerStatus();
		void activateIdleWorkers();
		void trainAdditionalWorkers();
		void onFrame();
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);

		void setScout(BWAPI::Unit unit);
		void setMineralWorker(BWAPI::Unit unit);
		void setGasWorker(BWAPI::Unit unit);
		void setBuildingWorker(BWAPI::Unit unit, WorkerData::BuildJob buildJob);

		void updateIdleBuildWorker(BWAPI::Unit worker);
		void updateIdleScout(BWAPI::Unit worker);

		//BWAPI::Unit getBuilder(Building& b, bool setJobAsBuilder = true);
		//BWAPI::Unit getMoveWorker(BWAPI::Position p);
		BWAPI::Unit getClosestDepot(BWAPI::Unit worker);
		//BWAPI::Unit getGasWorker(BWAPI::Unit refinery);
		//BWAPI::Unit getClosestEnemyUnit(BWAPI::Unit worker);
		//BWAPI::Unit getClosestMineralWorkerTo(BWAPI::Unit enemyUnit);
		BWAPI::Unit getWorkerScout();
		BWAPI::Position getScoutPosition(BWAPI::Unit scout);
		BWAPI::Unit getBuilder(BWAPI::UnitType type, BWAPI::Position pos);
		BWAPI::Unit getWorker();
		WorkerData getWorkerData();

		std::vector<WorkerData::BuildJob> getActiveBuildJobs();
		std::vector<WorkerData::BuildJob> getActiveBuildJobs(BWAPI::UnitType unitType);
	};
}
