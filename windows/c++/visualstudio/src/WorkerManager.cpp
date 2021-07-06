#include "WorkerManager.h"

#include <BWAPI/Client/Client.h>
#include "Global.h"
#include "BWEM/src/bwem.h"

#include "Tools.h"
using namespace MiraBot;

WorkerManager::WorkerManager()
{
}


void WorkerManager::onFrame()
{
	// Update workers, e.g. set job as 'Idle' when job completed
	updateWorkerStatus();

	// Send idle workers to gather resources
	activateIdleWorkers();

	// Send out a new scout if needed
	if (BWAPI::Broodwar->getFrameCount() % 7919 == 0 && should_have_new_scout)
	{
		auto* new_scout = getAnyWorker();
		m_workerData.setWorkerJob(new_scout, WorkerData::Scout, scout_last_known_position);
		should_have_new_scout = false;
	}


	// Hotfix
	while (Global::combat().m_attack_units.size() > 30 && m_workerData.getWorkers(WorkerData::Repair).size() < 6)
	{
		if (m_workerData.getWorkers(WorkerData::Minerals).size() <= 40) break;

		std::cout << "Making late game scout\n";
		m_workerData.setLateGameScout(getAnyWorker());
	}

	/*if (m_workerData.getWorkers(WorkerData::Minerals).empty())
	{
		for (auto s : m_workerData.getWorkers(WorkerData::Repair))
		{
			std::cout << "TEST\n";
			setMineralWorker(s);
		}
	}*/

	for (auto s : m_workerData.getWorkers(WorkerData::Repair))
	{
		//if (s->isStuck())
		//{
		//	//s->stop();
		//	m_workerData.setLateGameScout(getAnyWorker());
		//}
	}
}

void WorkerManager::updateWorkerStatus()
{
	for (const auto& worker : m_workerData.getWorkers())
	{
		const auto job = m_workerData.getWorkerJob(worker);
		if (!worker->isCompleted()) continue;


		// Workers can be idle for various reasons, e.g. a builder waiting to build
		if (worker->isIdle() || job == WorkerData::Scout && worker->isStuck()
		)
		{
			switch (job)
			{
			case WorkerData::Build:
				{
					/* Builder is idle when it has reached the build location and is waiting to build,
					 * or it has gotten stuck along the way*/
					handleIdleBuildWorker(worker);
					break;
				}
			case WorkerData::Scout:
				{
					// Scout is idle when it has reached its current scouting target
					handleIdleScout(worker);
					break;
				}
			case WorkerData::Repair:
				{
					auto random_pos = Global::map().map.RandomPosition();
					auto pos = BWAPI::Position(Global::production().m_building_placer_.getBuildLocationNear(
						BWAPI::TilePosition(random_pos), BWAPI::UnitTypes::Protoss_Nexus));
					worker->move(pos);
					break;
				}
			case WorkerData::Move:
				{
					// Mover is idle when it has reached its destination
					break;
				}
			default:
				{
					if (!getWorkerScout() && getScoutPosition(worker)) setScout(worker);
					else m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
				}
			}
		}
	}
}

void WorkerManager::setScout(BWAPI::Unit unit)
{
	m_workerData.setWorkerJob(unit, WorkerData::Scout, getScoutPosition(unit));
}


// Assign jobs to idle workers
// TODO: 2 workers per mineral patch is optimal
// TODO: Look at total minerals and gas - e.g. maybe we have plenty of gas but no minerals
void WorkerManager::activateIdleWorkers()
{
	const auto mineral_worker_count = m_workerData.getWorkers(WorkerData::Minerals).size();
	const auto gas_worker_count = m_workerData.getWorkers(WorkerData::Gas).size();

	for (auto& worker : m_workerData.getWorkers())
	{
		if (m_workerData.getWorkerJob(worker) == WorkerData::Idle)
		{
			const auto refinery_type = BWAPI::Broodwar->self()->getRace().getRefinery();
			const auto refinery_count = Tools::countUnitsOfType(refinery_type);

			// We don't want more than 3 gas workers per refinery
			if (refinery_count > 0 && gas_worker_count < mineral_worker_count / 4
				&& gas_worker_count < Tools::countUnitsOfType(refinery_type) * 4)
				setGasWorker(worker);
			else setMineralWorker(worker);
		}
	}
}

void WorkerManager::setMineralWorker(BWAPI::Unit unit)
{
	BWAPI::Unit closestDepot = getClosestDepot(unit);
	if (closestDepot)
	{
		m_workerData.setWorkerJob(unit, WorkerData::Minerals, closestDepot);
	}
}

void WorkerManager::setGasWorker(BWAPI::Unit unit)
{
	BWAPI::Unit closestDepot = getClosestDepot(unit);
	if (closestDepot)
	{
		m_workerData.setWorkerJob(unit, WorkerData::Gas, closestDepot);
	}
}

void WorkerManager::setBuildingWorker(BWAPI::Unit unit, WorkerData::BuildJob buildJob)
{
	m_workerData.setWorkerJob(unit, WorkerData::Build, buildJob);
}


void WorkerManager::onUnitComplete(BWAPI::Unit unit)
{
	if (!unit->getType().isWorker()) return;
	updateWorkerCounts();
	m_workerData.addWorker(unit, WorkerData::Idle, nullptr);
}

void WorkerManager::onUnitDestroy(BWAPI::Unit unit)
{
	m_workerData.workerDestroyed(unit);
}

void WorkerManager::updateWorkerCounts()
{
	auto num_patches = 0;
	auto num_geysers = 0;
	for (const auto* base : Global::map().expos)
	{
		num_patches += base->Minerals().size();
		num_geysers += base->Geysers().size();
	}
	//max_workers = num_patches * 3 + m_workerData.getWorkers(WorkerData::Scout).size() + 5;
	max_workers = 65;
}

BWAPI::Unit WorkerManager::getClosestDepot(BWAPI::Unit worker)
{
	BWAPI::Unit closestDepot = nullptr;
	double closestDistance = 0;
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().isResourceDepot() && unit->isCompleted())
		{
			const double distance = unit->getDistance(worker);
			if (!closestDepot || distance < closestDistance)
			{
				closestDepot = unit;
				closestDistance = distance;
			}
		}
	}
	return closestDepot;
}


// Returns next scouting location for scout, favoring closest locations
BWAPI::Position WorkerManager::getScoutPosition(BWAPI::Unit scout)
{
	// if we found enemy, we don't have more positions
	//if (Global::information().enemy_start_location) return BWAPI::Positions::None;

	auto& startLocations = BWAPI::Broodwar->getStartLocations();

	BWAPI::Position closestPosition = BWAPI::Positions::None;
	double closestDistance = 0;

	for (BWAPI::TilePosition position : startLocations)
	{
		auto pos = BWAPI::Position(position);
		if (!BWAPI::Broodwar->isExplored(position) || (Global::map().lastSeen(position) > 5000 && !BWAPI::Broodwar->
			isVisible(position)))
		{
			const BWAPI::Position pos(position);
			const double distance = scout->getDistance(pos);

			if (!closestPosition || distance < closestDistance)
			{
				closestPosition = pos;
				closestDistance = distance;
			}
		}
	}
	return closestPosition;
}

// Returns the scout
BWAPI::Unit WorkerManager::getWorkerScout()
{
	for (auto& worker : m_workerData.getWorkers())
	{
		if (!worker) continue;
		if (m_workerData.getWorkerJob(worker) == WorkerData::Scout)
		{
			return worker;
		}
	}

	return nullptr;
}

// Looks through mineral workers and returns the closest candidate to given position
BWAPI::Unit WorkerManager::getBuilder(BWAPI::UnitType type, BWAPI::Position pos)
{
	BWAPI::Unit closestUnit = nullptr;
	auto unitSet = m_workerData.getWorkers(WorkerData::Minerals);
	for (auto& unit : unitSet)
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

// Returns a worker: used when you just need a worker asap
BWAPI::Unit WorkerManager::getAnyWorker(BWAPI::Position pos)
{
	// Prioritized job order: Prefers Idle, Default, Minerals, ..., Combat, Repair
	for (auto& job : m_workerData.prioritized_jobs)
	{
		const auto workers = m_workerData.getWorkers(job);
		if (workers.empty()) continue;

		// If position doesn't matter, use first found candidate
		if (pos == BWAPI::Positions::None)
		{
			for (auto& unit : workers) return unit;
		}

		// Return closest candidate
		auto sorted_workers = Tools::sortUnitsByClosest(pos, workers);
		return sorted_workers[0];
	}

	return nullptr;
}

// Handles a build-worker who is idling
void WorkerManager::handleIdleBuildWorker(BWAPI::Unit worker)
{
	// Get build job assigned to worker
	const auto building_type = m_workerData.m_workerBuildingTypeMap[worker];
	auto building_pos = m_workerData.m_buildPosMap[worker];
	auto initiatedBuilding = m_workerData.m_initiatedBuildingMap[worker];

	if (building_type == BWAPI::UnitTypes::Protoss_Nexus)
	{
		int i = 0;
	}

	// If worker idling because build job is completed, set job to idle
	if (initiatedBuilding && !worker->isConstructing())
	{
		if (Tools::getUnitsOfType(building_type, false, false).empty())
		{
		}
		else
		{
			m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
			return;
		}
	}
	if (building_type == BWAPI::UnitTypes::None || worker->isConstructing())
	{
	}

	// Otherwise, worker is idling because it is waiting for the required resources
	if (building_type.mineralPrice() > 0 && building_type.mineralPrice() + 10 > BWAPI::Broodwar->self()->minerals())
		return;
	if (building_type.gasPrice() > 0 && building_type.gasPrice() + 10 > BWAPI::Broodwar->self()->gas()) return;

	// Check build position validity
	auto canbuild = BWAPI::Broodwar->canBuildHere(building_pos, building_type, worker);
	if (!canbuild)
	{
		if (building_pos != BWAPI::TilePositions::None)
		{
			building_pos = Global::production().m_building_placer_.getBuildLocationNear(building_pos, building_type);
			canbuild = BWAPI::Broodwar->canBuildHere(building_pos, building_type, worker);
			if (canbuild)
			{
				goto sup;
			}
		}

		// First attempt: Build position based on builder position
		building_pos = Global::production().m_building_placer_.getBuildLocationNear(
			worker->getTilePosition(), building_type);
		canbuild = BWAPI::Broodwar->canBuildHere(building_pos, building_type, worker);
		if (canbuild)
		{
			goto sup;
		}

		// Second attempt: Build position using built-in StarCraft building placer
		building_pos = BWAPI::Broodwar->getBuildLocation(building_type, worker->getTilePosition(), 400);
		canbuild = BWAPI::Broodwar->canBuildHere(building_pos, building_type, worker);
		if (canbuild)
		{
			goto sup;
		}

		// Final attempt: Get any damn build position
		building_pos = Global::production().m_building_placer_.getBuildLocation(building_type);
		canbuild = BWAPI::Broodwar->canBuildHere(building_pos, building_type, worker);
		if (canbuild)
		{
			goto sup;
		}

		m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
		return;
	}
sup:


	// Try to place the building and generate new position if the first one fails
	bool built = worker->build(building_type, building_pos);
	if (!built)
	{
		std::cout << "Failed to build " << building_type.getName() << "\n";
		m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);

		return;
	}

	// Used in the beginning of the function next frame
	//m_workerData.m_workerBuildingTypeMap[worker] = BWAPI::UnitTypes::None;
	m_workerData.m_initiatedBuildingMap[worker] = true;
	//BWAPI::Broodwar->printf("Building %s", building_type.getName().c_str());
}

// Send worker to unexplored scout position
void WorkerManager::handleIdleScout(BWAPI::Unit worker)
{
	const auto scout_position = getScoutPosition(worker);
	if (!scout_position)
	{
		auto enemy_area = Global::map().map.GetNearestArea(Global::information().enemy_start_location);
		auto top_right = BWAPI::Position(BWAPI::TilePosition(enemy_area->BottomRight().x, enemy_area->TopLeft().y));
		auto bottom_left = BWAPI::Position(BWAPI::TilePosition(enemy_area->TopLeft().x, enemy_area->BottomRight().y));
		auto top_left = BWAPI::Position(enemy_area->TopLeft());
		auto bottom_right = BWAPI::Position(enemy_area->BottomRight());

		auto go_to_pos = top_left;
		// All starting positions have been explored
		// If worker is away from enemy base keep going in and out
		if (top_left.getApproxDistance(worker->getPosition()) <= 400)
		{
			go_to_pos = top_right;
		}
		else if (top_right.getApproxDistance(worker->getPosition()) <= 400)
		{
			go_to_pos = bottom_right;
		}
		else if (bottom_right.getApproxDistance(worker->getPosition()) <= 400)
		{
			go_to_pos = bottom_left;
		}
		else if (bottom_left.getApproxDistance(worker->getPosition()) <= 400)
		{
			go_to_pos = top_left;
		}
		m_workerData.setWorkerJob(worker, WorkerData::Scout,
		                          BWAPI::Position(BWAPI::WalkPosition(go_to_pos)));
	}
	else
	{
		// Set scout's new target position
		m_workerData.setWorkerJob(worker, WorkerData::Scout, scout_position);
	}
}


// Returns a vector of all active (=unfinished) build jobs
std::vector<WorkerData::BuildJob> WorkerManager::getActiveBuildJobs()
{
	std::vector<WorkerData::BuildJob> build_jobs;

	auto builders = m_workerData.getWorkers(WorkerData::Build);
	for (const auto& unit : builders)
	{
		auto unit_building_type = m_workerData.m_workerBuildingTypeMap[unit];
		if (unit_building_type == BWAPI::UnitTypes::None) continue;
		// TODO: safer way to check map, this can cause exceptions - use find instead

		auto build_job = WorkerData::BuildJob{m_workerData.m_buildPosMap[unit], unit_building_type};
		build_jobs.push_back(build_job);
	}
	return build_jobs;
}

// Returns a vector of active (=unfinished) build jobs of given unit type
std::vector<WorkerData::BuildJob> WorkerManager::getActiveBuildJobs(BWAPI::UnitType unit_type)
{
	std::vector<WorkerData::BuildJob> build_jobs;
	auto builders = m_workerData.getWorkers(WorkerData::Build);
	for (const auto& unit : builders)
	{
		// If unit has build job for this unit type, add to vector
		const auto it = m_workerData.m_workerBuildingTypeMap.find(unit);
		if (it != m_workerData.m_workerBuildingTypeMap.end() && it->second == unit_type)
		{
			auto build_job = WorkerData::BuildJob{m_workerData.m_buildPosMap[unit], unit_type};
			build_jobs.push_back(build_job);
		}
	}
	return build_jobs;
}
