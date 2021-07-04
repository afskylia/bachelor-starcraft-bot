#pragma once

#include <BWAPI.h>
#include "BWEM/src/bwem.h"

// Made with inspiration from WorkerManager and WorkerData in UAlbertaBot

namespace MiraBot
{
	class WorkerData
	{
	public:
		enum WorkerJob { Minerals, Gas, Build, Combat, Idle, Repair, Move, Scout, Default };

		WorkerJob prioritized_jobs[9] = {Idle, Default, Minerals, Gas, Move, Scout, Combat, Repair};

		// initialized like this:  struct BuildJob move = {tilepos, unittype}
		struct BuildJob
		{
			BWAPI::TilePosition tilePos;
			BWAPI::UnitType unitType; // null if nothing to be built
		};

		void addWorker(BWAPI::Unit unit);
		void addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Unit jobUnit);
		void addWorker(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Position pos);
		void addWorker(BWAPI::Unit unit, enum WorkerJob job, struct BuildJob buildJob);

		void setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Unit jobUnit);
		void setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Position pos);
		void setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, struct BuildJob buildJob);

		void resetJob(BWAPI::Unit unit);
		void workerDestroyed(BWAPI::Unit unit);
		void addDepot(BWAPI::Unit unit);
		void removeDepot(BWAPI::Unit unit);

		BWAPI::Unitset getWorkers();
		BWAPI::Unitset getWorkers(WorkerJob job);

		bool depotIsFull(BWAPI::Unit depot);
		int getMineralsNearDepot(BWAPI::Unit depot);

		WorkerJob getWorkerJob(BWAPI::Unit unit);
		//MoveData		getWorkerMoveData(BWAPI::Unit unit);
		BWAPI::UnitType getWorkerBuildingType(BWAPI::Unit unit);
		BWAPI::Unit getWorkerDepot(BWAPI::Unit unit);
		const BWEM::Mineral* getMineralToMine(BWAPI::Unit unit);
		BWAPI::Unit getClosestRefinery(BWAPI::Unit unit);
		BWAPI::Unit getBuilder(BWAPI::UnitType buildingType, BWAPI::Position position);

		BWAPI::Unitset m_workers;
		BWAPI::Unitset m_depots;

		std::map<BWAPI::Unit, enum WorkerJob> m_workerJobMap;
		std::map<BWAPI::Unit, BWAPI::Unit> m_workerDepotMap;
		std::map<BWAPI::Unit, const BWEM::Mineral*> m_workerMineralMap; // Mineral to be collected from by worker
		std::map<BWAPI::Unit, BWAPI::Unit> m_workerRefineryMap; // Vespene gas geyser to be collected from by worker
		std::map<BWAPI::Unit, BWAPI::Unit> m_workerRepairMap; // Unit that repairworker has to repair
		std::map<BWAPI::Unit, BWAPI::Position> m_workerMoveMap;
		// Target position of e.g. a scout, or position where a builder has to build something
		std::map<BWAPI::Unit, BWAPI::TilePosition> m_buildPosMap;
		// Tile position where the unit should place a building
		std::map<BWAPI::Unit, BWAPI::UnitType> m_workerBuildingTypeMap; // Type of building to be built by builder
		std::map<BWAPI::Unit, const BWEM::Area*> m_workerAreaMap;
		std::map<BWAPI::Unit, bool> m_initiatedBuildingMap;

		std::map<const BWEM::Mineral*, int> m_workersOnMineralPatch;

		std::map<BWAPI::Unit, int> m_depotWorkerCount;
		std::map<BWAPI::Unit, int> m_refineryWorkerCount;
		std::map<BWAPI::Unit, BWAPI::Unit> m_workerMineralAssignment;
	};
}
