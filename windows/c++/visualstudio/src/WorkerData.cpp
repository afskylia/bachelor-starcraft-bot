#include "WorkerData.h"


#include "MiraBot.h"
#include "Tools.h"


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
void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, struct MoveData moveData)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	setWorkerJob(unit, job, moveData);
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

// Set worker job with move position (e.g. Scout, Move or Build)
void WorkerData::setWorkerJob(BWAPI::Unit unit, enum WorkerJob job, struct MoveData moveData)
{
	if (!unit) { return; }
	m_workerJobMap[unit] = job;
	m_workerMoveMap[unit] = moveData.position;
	unit->move(moveData.position);

	// If worker is Builder, map building type to worker
	if (job == Build) m_workerBuildingTypeMap[unit] = moveData.unitType;
	// TODO: enqueue build order?
}

BWAPI::Unit WorkerData::getMineralToMine(BWAPI::Unit unit)
{
	auto minerals_near_base = MiraBot::mainBase->getUnitsInRadius(1024, BWAPI::Filter::IsMineralField);
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