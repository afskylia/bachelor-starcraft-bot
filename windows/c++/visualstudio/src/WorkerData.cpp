#include "WorkerData.h"

#include "Global.h"
#include "MiraBotMain.h"
#include "Tools.h"

using namespace MiraBot;

// Add jobless worker
void WorkerData::addWorker(BWAPI::Unit unit)
{
	if (!unit) { return; }

	m_workers.insert(unit);
	m_workerAreaMap[unit] = Global::map().main_area;
	m_workerJobMap[unit] = Default;
}

// Add worker with job and job unit (e.g. Minerals or Repair jobs)
void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Unit jobUnit)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	m_workerAreaMap[unit] = Global::map().main_area;
	setWorkerJob(unit, job, jobUnit);
}

// Add worker with job and move position (e.g. Build, Scout or Move)
void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, BWAPI::Position pos)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	m_workerAreaMap[unit] = Global::map().main_area;
	setWorkerJob(unit, job, pos);
}

void WorkerData::addWorker(BWAPI::Unit unit, WorkerJob job, struct BuildJob buildJob)
{
	if (!unit) { return; }
	m_workers.insert(unit);
	m_workerAreaMap[unit] = Global::map().main_area;
	setWorkerJob(unit, job, buildJob);
}

// Called when a worker dies, certain jobs should be re-assigned
void WorkerData::workerDestroyed(BWAPI::Unit unit)
{
	// Try to re-assign build jobs
	if (m_workerJobMap[unit] == Build)
	{
		auto type = m_workerBuildingTypeMap[unit];
		auto move = m_workerMoveMap[unit];
		auto pos = m_buildPosMap[unit];
		auto newWorker = getBuilder(type, move);
		if (newWorker) setWorkerJob(newWorker, Build, BuildJob{pos, type});
	}

	// When scout dies
	if (m_workerJobMap[unit] == Scout)
	{
		auto new_scout = Global::workers().getAnyWorker();
		if (new_scout) setWorkerJob(new_scout, Scout, unit->getLastCommand().getTargetPosition());
	}

	resetJob(unit);
	m_workers.erase(unit);
}

void WorkerData::resetJob(BWAPI::Unit unit)
{
	WorkerJob previousJob = getWorkerJob(unit);

	switch (previousJob)
	{
	case Minerals:
		{
			//m_depotWorkerCount[m_workerDepotMap[unit]] -= 1; // TODO depots??

			auto mineral = m_workerMineralMap[unit];
			m_workersOnMineralPatch[mineral]--;

			m_workerDepotMap.erase(unit);
			m_workerMineralMap.erase(unit);
			break;
		}
	case Gas:
		{
			m_workerDepotMap.erase(unit);
			m_workerRefineryMap.erase(unit);
			break;
		}
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
			// Get available mineral in unit's area
			auto mineral_to_mine = getMineralToMine(unit);

			// TODO: Make function to switch base
			// If no available minerals in main base, go to 2nd base
			if (!mineral_to_mine && !(m_workerAreaMap[unit] == Global::map().snd_area))
			{
				m_workerAreaMap[unit] = Global::map().snd_area;
				mineral_to_mine = getMineralToMine(unit);
			}

			if (!mineral_to_mine)
			{
				//std::cout << "No available minerals in 2nd area\n";
				return;
			}

			m_workerMineralMap[unit] = mineral_to_mine;
			m_workersOnMineralPatch[mineral_to_mine]++;
			unit->gather(mineral_to_mine);
			break;
		}

	case Gas:
		{
			std::cout << "Sending unit to refinery\n";
			BWAPI::Unit refinery = getClosestRefinery(unit);
			m_workerRefineryMap[unit] = refinery;
			unit->gather(refinery);
			break;
		}
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
	// TODO get mineral in area, if no more in main area go to snd area
	// TODO make sure at most 2 units are assigned to patch

	auto minerals_in_base = m_workerAreaMap[unit]->Minerals();
	if (minerals_in_base.empty()) return nullptr;

	//auto minerals_near_base = Global::information().main_base->getUnitsInRadius(1024, BWAPI::Filter::IsMineralField);
	//auto sorted_minerals = Tools::sortUnitsByClosest(unit, minerals_in_base);

	BWAPI::Unit closest_mineral = nullptr;
	auto closest_distance = INT_MAX;
	for (auto mineral : minerals_in_base)
	{
		// We want at most 3 workers per mineral patch
		if (m_workersOnMineralPatch[mineral->Unit()] >= 3) continue; // TODO change to 3

		// Set initially closest unit
		if (!closest_mineral)
		{
			closest_mineral = mineral->Unit();
			continue;
		}

		// Compare size of mineral to currently closest mineral
		const auto distance = mineral->Unit()->getDistance(unit);
		if (distance >= closest_distance) continue;

		// Set closest mineral
		closest_mineral = mineral->Unit();
		closest_distance = distance;
	}

	return closest_mineral;
}

BWAPI::Unit WorkerData::getClosestRefinery(BWAPI::Unit unit)
{
	auto all_units = BWAPI::Broodwar->self()->getUnits();
	auto sorted_units = Tools::sortUnitsByClosest(unit, all_units);
	for (auto u : sorted_units)
	{
		if (u->getType().isRefinery()) // TODO: Check number of workers on the refinery?
		{
			return u;
		}
	}
	return nullptr;
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

// Returns workers with a certain job
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
		if (!closestUnit)
		{
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
