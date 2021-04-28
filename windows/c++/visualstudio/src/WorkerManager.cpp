#include "WorkerManager.h"

#include <BWAPI/Client/Client.h>


#include "Tools.h"

WorkerManager::WorkerManager()
{
}

void WorkerManager::onFrame()
{

	updateWorkerStatus();

	// Train more workers so we can gather more income
	trainAdditionalWorkers();


	// Send our idle workers to mine minerals so they don't just stand there
	sendIdleWorkersToMinerals();
}

void WorkerManager::updateWorkerStatus()
{
	for (auto& worker : m_workerData.getWorkers())
	{
		if (!worker->isCompleted())
		{
			continue;
		}

		if (worker->isIdle())
		{
			switch (m_workerData.getWorkerJob(worker))
			{
			case WorkerData::Build:
			{
				// TODO: build if we are in the building position
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
				if (!scoutPosition) {
					// All starting positions have been explored
					m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
				}
				else {
					// Set scout's new target position
					m_workerData.setWorkerJob(worker, WorkerData::Scout, WorkerData::MoveData{ scoutPosition });
				}
				break;
			}
			default:
			{
				m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
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
			if (!getWorkerScout() && getScoutPosition(worker))
			{
				WorkerData::MoveData move{ getScoutPosition(worker) };
				m_workerData.setWorkerJob(worker, WorkerData::Scout, move);
			}
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