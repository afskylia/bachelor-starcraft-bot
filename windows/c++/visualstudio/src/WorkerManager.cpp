#include "WorkerManager.h"

#include <BWAPI/Client/Client.h>


#include "MiraBot.h"
#include "Tools.h"

WorkerManager::WorkerManager()
{
}

void WorkerManager::onFrame()
{
	// Send scout ASAP

	updateWorkerStatus();
	//sendScout();

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

		// Set idle workers' job to Idle, unless job is Build, Move or Scout
		if (worker->isIdle() &&
			(m_workerData.getWorkerJob(worker) != WorkerData::Build) &&
			(m_workerData.getWorkerJob(worker) != WorkerData::Move) &&
			(m_workerData.getWorkerJob(worker) != WorkerData::Scout))
		{
			m_workerData.setWorkerJob(worker, WorkerData::Idle, nullptr);
		}
	}
}


void WorkerManager::sendIdleWorkersToMinerals()
{
	for (auto& worker : m_workerData.getWorkers())
	{
		if (m_workerData.getWorkerJob(worker) == WorkerData::Idle)
		{
			setMineralWorker(worker);
		}
	}


	// Let's send all of our starting workers to the closest mineral to them
	// First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
	const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
	for (auto& unit : myUnits)
	{
		// Check the unit type, if it is an idle worker, then we want to send it somewhere
		if (unit->getType().isWorker() && unit->isIdle())
		{
			//unit->gather(unit->getClosestUnit(BWAPI::Filter::IsMineralField));
			BWAPI::Unitset minerals_near_base = MiraBot::mainBase->getUnitsInRadius(1024, BWAPI::Filter::IsMineralField);

			auto sorted_minerals = Tools::SortUnitsByClosest(unit, minerals_near_base);
			for (auto m : sorted_minerals)
			{
				if (!m->isBeingGathered())
				{
					unit->gather(m);
					return;
				}
			}
			//unit->gather(Tools::GetClosestUnitTo(MiraBot::mainBase->getPosition(), minerals_near_base)); // TODO gather could fail if to many workers are on it
			unit->gather(sorted_minerals[0]);
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

/**
 * \brief Sends one Scout to figure out enemy location and race
 */
void WorkerManager::sendScout()
{
	// Get a scout
	if (!MiraBot::m_scout) {
		MiraBot::m_scout = Tools::GetUnitOfType(BWAPI::Broodwar->self()->getRace().getWorker());
	}

	auto& startLocations = BWAPI::Broodwar->getStartLocations();

	// Loop through all starting positions
	for (BWAPI::TilePosition position : startLocations)
	{
		if (!BWAPI::Broodwar->isExplored(position)) {
			const BWAPI::Position pos(position);

			auto command = MiraBot::m_scout->getLastCommand();
			if (command.getTargetPosition() == pos) { return; }

			MiraBot::m_scout->move(pos);
			return;
		}
	}
	// Return to home
	//m_scout->move(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
}

void WorkerManager::onUnitCreate(BWAPI::Unit unit)
{
	// TODO: Decide which job is needed right now
	m_workerData.addWorker(unit, WorkerData::Idle, MiraBot::mainBase);
}

void WorkerManager::onUnitDestroy(BWAPI::Unit unit)
{
	m_workerData.workerDestroyed(unit);
}
