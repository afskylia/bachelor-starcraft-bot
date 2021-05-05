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
	if (!unit) { return; }
	// TODO: Clear job
	m_workers.erase(unit);
}


// Set worker job with associated job unit (depot)
void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit) { return; }
	// TODO clearpreviousjob?

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
	m_workerJobMap[unit] = job;
	m_workerMoveMap[unit] = pos;

	// Send unit to position
	unit->move(pos);
}


// Set builder job
void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, struct BuildJob buildJob)
{
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

const BWAPI::Unitset& WorkerData::getWorkers() const
{
	return m_workers;
}

const BWAPI::Unitset& WorkerData::getWorkers(WorkerJob job) const
{
	BWAPI::Unitset workers = {};
	for (auto& unit : m_workers)
	{
		if (m_workerJobMap.at(unit) == job)
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