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
	sendScout();
	
	// Train more workers so we can gather more income
	trainAdditionalWorkers();

	// Send our idle workers to mine minerals so they don't just stand there
	sendIdleWorkersToMinerals();
}

void WorkerManager::sendIdleWorkersToMinerals()
{
	// Let's send all of our starting workers to the closest mineral to them
	// First we need to loop over all of the units that we (BWAPI::Broodwar->self()) own
	const BWAPI::Unitset& myUnits = BWAPI::Broodwar->self()->getUnits();
	for (auto& unit : myUnits)
	{
		// Check the unit type, if it is an idle worker, then we want to send it somewhere
		if (unit->getType().isWorker() && unit->isIdle())
		{
			std::cout << "Idle" << "\n";
			//unit->gather(unit->getClosestUnit(BWAPI::Filter::IsMineralField));
			unit->gather(Tools::GetClosestUnitTo(BWAPI::Position(MiraBot::mainBase), BWAPI::Broodwar->getMinerals())); // TODO gather could fail if to many workers are on it


			//// Get the closest mineral to this worker unit
			//const BWAPI::Unit closestMineral = Tools::GetClosestUnitTo(unit, BWAPI::Broodwar->getMinerals());

			//// If a valid mineral was found, right click it with the unit in order to start harvesting
			//if (closestMineral) { unit->gather(closestMineral); }
		}
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

