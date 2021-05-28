#include "ProductionManager.h"

#include "Global.h"
#include "Tools.h"
#include "WorkerManager.h"

using namespace MiraBot;
/*
 * Everything production related
 */
ProductionManager::ProductionManager()
{
	for (int i = 0; i <= 200; ++i)
	{
		// TODO: Revamp of build order system so it doesn't have probes on empty levels
		m_build_order_[i] = BWAPI::UnitTypes::Protoss_Probe;
	}
	m_build_order_[4] = BWAPI::UnitTypes::Protoss_Nexus;
	m_build_order_[8] = BWAPI::UnitTypes::Protoss_Pylon;
	m_build_order_[10] = BWAPI::UnitTypes::Protoss_Gateway;
	m_build_order_[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	m_build_order_[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
}


void ProductionManager::onFrame()
{
	// Enforce build order
	tryCompareUnitsAndBuild();

	// Make idle buildings produce units if needed
	activateIdleBuildings();

	// TODO: Instead, automatically add supply depots to build order when about to run out
	buildAdditionalSupply();
}

void ProductionManager::tryBuildOrTrainUnit()
{
	if (m_build_queue_.empty())
	{
		return;
	}
	// Access oldest element and remove it
	const BWAPI::UnitType unit_type = m_build_queue_.front();
	m_build_queue_.pop_front();

	// Try to build unit type
	if (unit_type.isBuilding())
	{
		const auto built = buildBuilding(unit_type);
		if (!built) m_last_build_frame_ = BWAPI::Broodwar->getFrameCount();
	}
	else
	{
		// TODO: revamp of build order system so it doesn't have probes on the empty spots?
		// This is a hotfix until then - ignores all "probe" levels from the build order
		if (unit_type == BWAPI::UnitTypes::Protoss_Probe) return;

		const auto built = trainUnit(unit_type);
		if (!built) m_last_build_frame_ = BWAPI::Broodwar->getFrameCount();
	}
}

bool ProductionManager::addToBuildQueue(const BWAPI::UnitType& unit_type)
{
	if (std::find(m_build_queue_.begin(), m_build_queue_.end(), unit_type) == m_build_queue_.end())
	{
		// Add unit to build queue and remove from build order
		m_build_queue_.push_back(unit_type);

		// TODO debug
		for (auto build_queue : m_build_queue_)
		{
			//std::cout << build_queue;
		}
		//std::cout << "\n";
		return true;
	}
	return false;
}

/**
 * @return map of all built units <number of units, UnitType>
 */
std::map<BWAPI::UnitType, int> ProductionManager::getMapOfAllUnits()
{
	std::map<BWAPI::UnitType, int> all_units;

	auto units = BWAPI::Broodwar->self()->getUnits();
	for (auto* unit : units)
	{
		all_units[unit->getType()]++;
	}
	return all_units;
}

/**
 * @return map of required units <number of units, UnitType>
 */
std::map<BWAPI::UnitType, int> ProductionManager::getMapOfRequiredUnits()
{
	std::map<BWAPI::UnitType, int> required_units;

	// Get next supply
	const int supply = (Tools::GetTotalUsedSupply(true) / 2);

	// Iterate build order
	for (auto build_order : m_build_order_)
	{
		// If unit needed to be build, add it to required_units
		if (build_order.first <= supply)
		{
			required_units[build_order.second]++;
		}
	}

	return required_units;
}

/**
 * Compare units with required units and build difference
 */
void ProductionManager::compareUnitsAndBuild()
{
	auto all_units = getMapOfAllUnits();
	auto required_units = getMapOfRequiredUnits();

	// TODO if maps are the same, return

	for (auto required_unit : required_units)
	{
		// Try find in all units
		auto it = all_units.find(required_unit.first);
		// If not found or we need more units, build it
		if (it == all_units.end())
		{
			addToBuildQueue(required_unit.first);
			break;
		}
		if (required_unit.second > it->second)
			addToBuildQueue(required_unit.first);
	}
	tryBuildOrTrainUnit();
}

/**
 * Try to compare units with required units and build difference.
 * This is not enforced on every frame of the game, since that would
 * lead to buildings being enqueued multiple times due to delays.
 */
void ProductionManager::tryCompareUnitsAndBuild()
{
	const int frame_count = BWAPI::Broodwar->getFrameCount();
	if (frame_count == m_last_build_frame_ + 20)
	{
		compareUnitsAndBuild();
	}
}


void ProductionManager::activateIdleBuildings()
{
	/**
	 * TODO: Make this better and less hardcoded
	 * Should count number of units w. certain job, not just all units.
	 * E.g. "we need 30 mineral workers, 20 gas workers" and so on.
	 * Also the number of units we need should dynamically be adjusted if needed.
	 *
	 * TODO: Maybe make a class (InformationManager?) that stores relevant information
	 * Such as all the races and their unit types, how many of each type/job we want etc.
	 * Then this function can iterate the list of needed units in a smart way.
	 *
	 * TODO: Only train units if we can afford it
	 * (I.e. it won't make us too poor to afford higher priority units/upgrades.)
	 */

	const auto worker_type = BWAPI::Broodwar->self()->getRace().getWorker();
	auto workers_owned = Tools::CountUnitsOfType(worker_type, BWAPI::Broodwar->self()->getUnits());
	const auto workers_wanted = 30;
	auto idle_nexuses = Tools::GetUnitsOfType(BWAPI::UnitTypes::Protoss_Nexus);
	while (workers_owned <= workers_wanted && !idle_nexuses.empty())
	{
		//if (unit->getType() == type && unit->isCompleted() && unit->isIdle())
		auto nexus = idle_nexuses.back();
		if (!nexus) return;
		nexus->train(worker_type);
		idle_nexuses.pop_back();
		workers_owned++;
	}

	// TODO: Find a way to efficiently do this for all types (instead of repetitive code):
	auto zealot_type = BWAPI::UnitTypes::Protoss_Zealot;
	auto zealots_owned = Tools::CountUnitsOfType(zealot_type, BWAPI::Broodwar->self()->getUnits());
	auto zealots_wanted = 30;
	auto idle_gateways = Tools::GetUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway);
	while (zealots_owned <= zealots_wanted && !idle_gateways.empty())
	{
		//if (unit->getType() == type && unit->isCompleted() && unit->isIdle())
		auto gateway = idle_gateways.back();
		if (!gateway) return;
		gateway->train(zealot_type);
		idle_gateways.pop_back();
		zealots_owned++;
	}
}


/**
 * Update build queue with destroyed units
 */
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	m_build_queue_.push_back(unit->getType());
}

/**
 * Remove trained or build unit
 */
void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
	m_last_build_frame_ = BWAPI::Broodwar->getFrameCount();
}

/**
 * Try to train unit
 */
bool ProductionManager::trainUnit(const BWAPI::UnitType& unit)
{
	// If we cannot afford unit
	if (unit.mineralPrice() > BWAPI::Broodwar->self()->minerals()) { return false; }
	switch (unit)
	{
	case BWAPI::UnitTypes::Protoss_Probe:
		{
			// get the unit pointer to my depot
			const BWAPI::Unit myDepot = Tools::GetDepot();

			// if we have a valid depot unit and it's currently not training something, train a worker
			// there is no reason for a bot to ever use the unit queueing system, it just wastes resources
			if (myDepot && !myDepot->isTraining())
			{
				myDepot->train(unit);
			}
			break;
		}
	default:
		{
			std::cout << unit << " not supported \n";
			return false;
		}
	}
	return true;
}

bool ProductionManager::trainUnit(const BWAPI::UnitType& unit, BWAPI::Unit depot)
{
	if (unit.mineralPrice() > BWAPI::Broodwar->self()->minerals()) { return false; }
	if (depot && !depot->isTraining())
	{
		depot->train(unit);
		return true;
	}
	return false;
}

/**
 *@deprecated use build order
 */
[[deprecated]]
void ProductionManager::buildGateway()
{
	const auto unitType = BWAPI::UnitTypes::Protoss_Gateway;
	if (countBuildings(unitType, true) < 4)
	{
		auto startedBuilding = buildBuilding(unitType);
	}
}

/**
 *@deprecated use build order
 */
[[deprecated]]
void ProductionManager::buildAttackUnits()
{
	const auto unitType = BWAPI::UnitTypes::Protoss_Zealot;
	auto gateways = Tools::GetUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway);
	for (auto* gateway : gateways)
	{
		if (gateway && !gateway->isTraining()) { gateway->train(unitType); }
	}
}

/**
 *@deprecated use build order
 */
[[deprecated]]
void ProductionManager::buildAdditionalSupply()
{
	// Get the amount of supply supply we currently have unused
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	// Only build one supply depot at a time
	if (pendingBuildingsCount(supplyProviderType) > 0) return;

	// If we have a sufficient amount of supply, we don't need to do anything
	if (BWAPI::Broodwar->self()->supplyUsed() + 8 >= Tools::GetTotalSupply(true))
	{
		// Otherwise, we are going to build a supply provider
		const auto startedBuilding = buildBuilding(supplyProviderType);
	}
}

int ProductionManager::countIdleBuildings(BWAPI::UnitType type, bool pending)
{
	return 0;
}


// Returns num. of owned buildings, optionally also pending ones
int ProductionManager::countBuildings(bool pending)
{
	int sum = std::size(BWAPI::Broodwar->self()->getUnits());
	if (pending) sum += pendingBuildingsCount();
	return sum;
}

// Returns num. of owned buildings of given type, optionally also pending ones
int ProductionManager::countBuildings(BWAPI::UnitType type, bool pending)
{
	auto sum = 0;
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() == type)sum++;
	}

	if (pending) sum += pendingBuildingsCount(type);
	return sum;
}

// Returns number of pending buildings (build job assigned but not yet built)
int ProductionManager::pendingBuildingsCount()
{
	auto buildJobs = Global::workers().getActiveBuildJobs();
	return std::size(buildJobs);
}

// Returns number of pending buildings of given type (build job assigned but not yet built)
int ProductionManager::pendingBuildingsCount(BWAPI::UnitType type)
{
	auto buildJobs = Global::workers().getActiveBuildJobs(type);
	return std::size(buildJobs);
}

// Return currently owned minerals, INCLUDING cost of pending build jobs
int ProductionManager::getTotalMinerals()
{
	auto totalMinerals = BWAPI::Broodwar->self()->minerals();

	for (auto& buildJob : Global::workers().getActiveBuildJobs())
	{
		totalMinerals -= buildJob.unitType.mineralPrice();
	}
	return totalMinerals;
}

// Return currently owned vespene gas, INCLUDING cost of pending build jobs
int ProductionManager::getTotalGas()
{
	// TODO
	return 0;
}

bool ProductionManager::buildBuilding(BWAPI::UnitType type)
{
	// TODO: Account for both minerals and gas
	// TODO: Does the unit type require multiple workers?

	// If we have much less minerals than required, it's not worth to wait for it
	// TODO: !!! Also look at pending mineral costs somehow
	if (Global::production().getTotalMinerals() < type.mineralPrice() * 0.7) return false;
	//if (BWAPI::Broodwar->self()->minerals() < type.mineralPrice() * 0.7) return false;

	// Get the type of unit that is required to build the desired building
	BWAPI::UnitType builderType = type.whatBuilds().first;

	// Get a location that we want to build the building next to
	BWAPI::TilePosition desiredPos = BWAPI::Broodwar->self()->getStartLocation();

	// Ask BWAPI for a building location near the desired position for the type
	int maxBuildRange = 64;
	bool buildingOnCreep = type.requiresCreep();
	BWAPI::TilePosition buildPos = BWAPI::Broodwar->getBuildLocation(type, desiredPos, maxBuildRange, buildingOnCreep);

	// Try to build the unit
	auto* builder = Global::workers().getBuilder(builderType, BWAPI::Position(buildPos));
	if (!builder) { return false; }

	Global::workers().setBuildingWorker(builder, WorkerData::BuildJob{buildPos, type});
	return true;
}
