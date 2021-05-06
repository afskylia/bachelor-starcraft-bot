#include "WorkerManager.h"

#include <BWAPI/Client/Client.h>
#include "Global.h"

#include "Tools.h"
using namespace MiraBot;

WorkerManager::WorkerManager()
{
}

void WorkerManager::onFrame()
{
	updateWorkerStatus();

	// Train more workers so we can gather more income
	// trainAdditionalWorkers();


	// Send our idle workers to mine minerals so they don't just stand there
	sendIdleWorkersToMinerals();
}

void WorkerManager::updateWorkerStatus()
{
	for (auto& worker : m_workerData.getWorkers())
	{
		auto job = m_workerData.getWorkerJob(worker);
		if (!worker->isCompleted())
		{
			continue;
		}

		if (worker->isIdle())
		{
			switch (job)
			{
			case WorkerData::Build:
				{
					// TODO:: Put this into its own function

					auto buildingType = m_workerData.m_workerBuildingTypeMap[worker];
					if (buildingType == BWAPI::UnitTypes::None)
					{
						m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
						break;
					}

					// TODO: Make sure we didn't get trapped somewhere, if we're not at the target we should walk there
					// TODO: What if worker can't reach the position? check if is walkable
					// Check if worker reached the goal position // TODO: Or is close enough!
					/*auto workerMove = m_workerData.m_workerMoveMap[worker];
					if (!Global::Map().isWalkable(workerMove.x, workerMove.y))
					{
						std::cout << "Whoops can't walk there\n";
						m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
						break;
					}*/

					// if worker->getPosition() == m_workerData.m_workerMoveMap[worker]
					if constexpr (true)
					{
						if (buildingType.mineralPrice() > BWAPI::Broodwar->self()->minerals()) break;

						auto buildingPos = m_workerData.m_buildPosMap[worker];


						// Try to place the building
						auto failCount = 0; // number of times we have tried to build
						while (!worker->build(buildingType, buildingPos) && failCount < 4)
						{
							failCount++;

							// Ask BWAPI for a new building location
							int maxBuildRange = 64;
							bool buildingOnCreep = buildingType.requiresCreep();
							buildingPos = BWAPI::Broodwar->getBuildLocation(
								buildingType, buildingPos, maxBuildRange, buildingOnCreep);
							std::cout << "Failed to build " << buildingType.getName() << "\n";
						}

						m_workerData.m_workerBuildingTypeMap[worker] = BWAPI::UnitTypes::None;
						std::cout << "Now building " << buildingType.getName() << "\n";
					}

					break;
				}
			case WorkerData::Move:
				{
					// TODO
					break;
				}
			case WorkerData::Scout:
				{
					auto scoutPosition = getScoutPosition(worker);
					if (!scoutPosition)
					{
						// All starting positions have been explored
						m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
					}
					else
					{
						// Set scout's new target position
						m_workerData.setWorkerJob(worker, WorkerData::Scout, scoutPosition);
					}
					break;
				}
			default:
				{
					m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
				}
			}
		}

		else
		{
			switch (job)
			{
			case WorkerData::Build:
				{
				}
			}
		}
	}
}


void WorkerManager::sendIdleWorkersToMinerals()
{
	for (auto& worker : m_workerData.getWorkers())
	{
		if (m_workerData.getWorkerJob(worker) == WorkerData::Idle)
		{
			// Send scout to unexplored scouting locations
			if (!getWorkerScout() && getScoutPosition(worker))
			{
				m_workerData.setWorkerJob(worker, WorkerData::Scout, getScoutPosition(worker));
			}

				// Idle workers gather minerals by default
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

void WorkerManager::setBuildingWorker(BWAPI::Unit unit, WorkerData::BuildJob buildJob)
{
	m_workerData.setWorkerJob(unit, WorkerData::Build, buildJob);
}

void WorkerManager::trainAdditionalWorkers()
{
	const BWAPI::UnitType workerType = BWAPI::Broodwar->self()->getRace().getWorker();
	const int workersWanted = 30;
	const int workersOwned = Tools::CountUnitsOfType(workerType, BWAPI::Broodwar->self()->getUnits());
	if (workersOwned < workersWanted)
	{
		// get the unit pointer to my depot
		const BWAPI::Unit myDepot = Tools::GetDepot();

		// if we have a valid depot unit and it's currently not training something, train a worker
		// there is no reason for a bot to ever use the unit queueing system, it just wastes resources
		if (myDepot && !myDepot->isTraining()) { myDepot->train(workerType); }
	}
}

void WorkerManager::onUnitCreate(BWAPI::Unit unit)
{
	// TODO: Decide which job is needed right now
	m_workerData.addWorker(unit, WorkerData::Idle, nullptr);
}

void WorkerManager::onUnitDestroy(BWAPI::Unit unit)
{
	m_workerData.workerDestroyed(unit);
}

BWAPI::Unit WorkerManager::getClosestDepot(BWAPI::Unit worker)
{
	BWAPI::Unit closestDepot = nullptr;
	double closestDistance = 0;
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		// TODO: && !m_workerData.depotIsFull(unit)
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
	auto& startLocations = BWAPI::Broodwar->getStartLocations();

	BWAPI::Position closestPosition = BWAPI::Positions::None;
	double closestDistance = 0;

	for (BWAPI::TilePosition position : startLocations)
	{
		if (!BWAPI::Broodwar->isExplored(position))
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

BWAPI::Unit WorkerManager::getWorkerScout()
{
	// for each of our workers
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

// Returns a vector of all active (=unfinished) build jobs
std::vector<WorkerData::BuildJob> WorkerManager::getActiveBuildJobs()
{
	std::vector<WorkerData::BuildJob> buildJobs;

	auto builders = m_workerData.getWorkers(WorkerData::Build);
	for (auto& unit : builders)
	{
		auto unitBuildingType = m_workerData.m_workerBuildingTypeMap[unit];
		if (unitBuildingType == BWAPI::UnitTypes::None) continue;
		// TODO: safer way to check map, this can cause exceptions - use find instead

		auto buildJob = WorkerData::BuildJob{m_workerData.m_buildPosMap[unit], unitBuildingType};
		buildJobs.push_back(buildJob);
	}
	return buildJobs;
}

// Returns a vector of active (=unfinished) build jobs of given unit type
std::vector<WorkerData::BuildJob> WorkerManager::getActiveBuildJobs(BWAPI::UnitType unitType)
{
	std::vector<WorkerData::BuildJob> buildJobs;
	auto builders = m_workerData.getWorkers(WorkerData::Build);
	for (auto& unit : builders)
	{
		auto unitBuildingType = m_workerData.m_workerBuildingTypeMap[unit];
		if (unitBuildingType == BWAPI::UnitTypes::None) continue;
		// TODO: safer way to check map, this can cause exceptions - use find instead

		if (unitBuildingType == unitType)
		{
			auto buildJob = WorkerData::BuildJob{m_workerData.m_buildPosMap[unit], unitType};
			buildJobs.push_back(buildJob);
		}
	}
	return buildJobs;
}
