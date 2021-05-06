#include "WorkerData.h"


#include "MiraBotMain.h"
#include "Tools.h"

using namespace MiraBot;

// Add jobless worker
void WorkerData::addWorker(BWAPI::Unit unit)
{
	if (!unit) { return; }

	m_workers.insert(unit);
	m_workerJobMap[unit] = Default;
}

// Add worker with job and job unit (e.g. Minerals or Repair jobs)
void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	setWorkerJob(unit, job, jobUnit);

}

// Add worker with job and move position (e.g. Build, Scout or Move)
void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Position pos)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	setWorkerJob(unit, job, pos);
}

void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, struct BuildJob buildJob)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	setWorkerJob(unit, job, buildJob);
}

void WorkerData::workerDestroyed(BWAPI::Unit unit)
{
	// TODO assign job to another worker (important if job==BUILD)
	if (!unit) { return; }
	resetJob(unit);
	m_workers.erase(unit);
}

void WorkerData::resetJob(BWAPI::Unit unit)
{
	if (!unit) { return; }
	WorkerJob previousJob = getWorkerJob(unit);

	switch (previousJob)
	{
	case Minerals:
	{
		//m_depotWorkerCount[m_workerDepotMap[unit]] -= 1; // TODO depots??
		m_workerDepotMap.erase(unit);
		m_workerMineralMap.erase(unit);
		break;
	}
	case Gas: break; // TODO: Gas!
	case Build:
	{
		m_workerBuildingTypeMap.erase(unit);
		m_workerMoveMap.erase(unit);
		m_buildPosMap.erase(unit);
		break;
	}
	case Move:
	{
		m_workerMoveMap.erase(unit);
		break;
	}
	case Scout:
	{
		m_workerMoveMap.erase(unit);
		break;
	}
	default: break;
	}

	m_workerJobMap.erase(unit);
}


// Set worker job with associated job unit (depot)
void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit) { return; }
	resetJob(unit);

	m_workerJobMap[unit] = job;
	m_workerDepotMap[unit] = jobUnit;

	switch (job)
	{
	case Minerals:
	{
		BWAPI::Unit mineralToMine = getMineralToMine(unit);
		m_workerMineralMap[unit] = mineralToMine;
		unit->gather(mineralToMine);
	}

	case Gas: break;
	case Combat: break;
	case Idle: break;
	case Repair: break;
	case Default: break;

	default: std::cout << "This should never happen\n";
	}
}

// Set worker job with move position (e.g. Scout or Move)
void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Position pos)
{
	if (!unit) { return; }
	resetJob(unit);
	
	m_workerJobMap[unit] = job;
	m_workerMoveMap[unit] = pos;

	// Send unit to position
	unit->move(pos);
}


// Set builder job
void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, struct BuildJob buildJob)
{
	if (!unit) { return; }
	resetJob(unit);
	
	m_workerJobMap[unit] = job;
	m_workerBuildingTypeMap[unit] = buildJob.unitType;
	m_workerMoveMap[unit] = BWAPI::Position(buildJob.tilePos);
	m_buildPosMap[unit] = buildJob.tilePos;

	// Send unit to position
	unit->move(m_workerMoveMap[unit]);
}

BWAPI::Unit WorkerData::getMineralToMine(BWAPI::Unit unit)
{
	auto minerals_near_base = MiraBotMain::mainBase->getUnitsInRadius(1024, BWAPI::Filter::IsMineralField);
	auto sorted_minerals = Tools::SortUnitsByClosest(unit, minerals_near_base);
	for (auto m : sorted_minerals)
	{
		if (!m->isBeingGathered())
		{
			return m;
		}
	}
	return sorted_minerals[0];
}

WorkerData::WorkerJob WorkerData::getWorkerJob(BWAPI::Unit unit)
{
	if (!unit) { return Default; }

	const auto it = m_workerJobMap.find(unit);
	if (it != m_workerJobMap.end())return it->second;
	return Default;
}

BWAPI::Unitset WorkerData::getWorkers()
{
	return m_workers;
}

BWAPI::Unitset WorkerData::getWorkers(WorkerJob job)
{
	BWAPI::Unitset workers = {};
	for (auto& unit : m_workers)
	{
		//if (m_workerJobMap.at(unit) == job)
		const auto it = m_workerJobMap.find(unit);
		if (it != m_workerJobMap.end() && it->second == job)
		{
			workers.insert(unit);
		}
	}
	return workers;
}

BWAPI::UnitType WorkerData::getWorkerBuildingType(BWAPI::Unit unit)
{
	if (!unit) { return BWAPI::UnitTypes::None; }

	std::map<BWAPI::Unit, BWAPI::UnitType>::iterator it = m_workerBuildingTypeMap.find(unit);

	if (it != m_workerBuildingTypeMap.end())
	{
		return it->second;
	}

	return BWAPI::UnitTypes::None;
}

BWAPI::Unit WorkerData::getBuilder(BWAPI::UnitType type, BWAPI::Position pos)
{
	BWAPI::Unit closestUnit = nullptr;
	for (auto& unit : getWorkers(Minerals))
	{

		// If worker isn't of required type or hasn't been trained yet, continue
		if (!(unit->getType() == type && unit->isCompleted())) continue;

		// Set initially closest worker
		if (!closestUnit) {
			closestUnit = unit;
			continue;
		}

		// If position doesn't matter, use the first found candidate
		if (pos == BWAPI::Positions::None) break;

		// Check if this unit is closer to the position than closestUnit
		if (closestUnit->getDistance(pos) > unit->getDistance(pos))
		{
			closestUnit = unit;
		}
	}

	// Return the closest worker, or nullptr if none was found
	return closestUnit;
}