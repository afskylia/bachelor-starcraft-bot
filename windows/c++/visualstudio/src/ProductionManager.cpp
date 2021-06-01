#include "ProductionManager.h"

#include "Global.h"
#include "Tools.h"
#include "WorkerManager.h"

using namespace MiraBot;
//using namespace BWAPI;
/*
 * Everything production related
 */
ProductionManager::ProductionManager()
{
	for (int i = 0; i <= 200; ++i)
	{
		// TODO: Revamp of build order system so it doesn't have probes on empty levels
		m_build_order[i] = BWAPI::UnitTypes::Protoss_Probe;
	}
	m_build_order[4] = BWAPI::UnitTypes::Protoss_Nexus;
	m_build_order[8] = BWAPI::UnitTypes::Protoss_Pylon;
	m_build_order[10] = BWAPI::UnitTypes::Protoss_Gateway;
	m_build_order[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	m_build_order[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
	m_build_order[15] = BWAPI::UnitTypes::Protoss_Gateway;
	m_build_order[20] = BWAPI::UnitTypes::Protoss_Gateway;
	m_build_order[25] = BWAPI::UnitTypes::Protoss_Gateway;
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
	if (m_build_queue.empty())
	{
		return;
	}
	// Access oldest element and remove it
	const BWAPI::UnitType unit_type = m_build_queue.front();
	m_build_queue.pop_front();

	// Try to build unit type
	if (unit_type.isBuilding())
	{
		const auto built = buildBuilding(unit_type);
		if (!built) m_last_build_frame = BWAPI::Broodwar->getFrameCount();
	}
	else
	{
		// TODO: revamp of build order system so it doesn't have probes on the empty spots?
		// This is a hotfix until then - ignores all "probe" levels from the build order
		if (unit_type == BWAPI::UnitTypes::Protoss_Probe) return;

		const auto built = trainUnit(unit_type);
		if (!built) m_last_build_frame = BWAPI::Broodwar->getFrameCount();
	}
}

bool ProductionManager::addToBuildQueue(const BWAPI::UnitType& unit_type)
{
	if (std::find(m_build_queue.begin(), m_build_queue.end(), unit_type) == m_build_queue.end())
	{
		// Add unit to build queue and remove from build order
		m_build_queue.push_back(unit_type);

		// TODO debug
		for (auto build_queue : m_build_queue)
		{
			//std::cout << build_queue;
		}
		//std::cout << "\n";
		//std::cout << "Adding " << unit_type << " to build queue\n";
		return true;
	}
	return false;
}

/**
 * @return map of all built units <number of units, UnitType>
 */
std::map<BWAPI::UnitType, int> ProductionManager::get_map_of_all_units()
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
std::map<BWAPI::UnitType, int> ProductionManager::get_map_of_required_units()
{
	std::map<BWAPI::UnitType, int> required_units;

	// Get next supply
	const int supply = (Tools::getTotalUsedSupply(true) / 2);

	// Iterate build order
	for (auto build_order : m_build_order)
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
	auto all_units = get_map_of_all_units();
	auto required_units = get_map_of_required_units();

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
	if (frame_count == m_last_build_frame + 20)
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
	auto workers_owned = Tools::countUnitsOfType(worker_type);
	const auto workers_wanted = 50;
	auto idle_nexuses = Tools::getUnitsOfType(BWAPI::UnitTypes::Protoss_Nexus, true);
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
	auto zealots_owned = Tools::countUnitsOfType(zealot_type);
	auto zealots_wanted = 30;
	auto idle_gateways = Tools::getUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, true);
	while (zealots_owned <= zealots_wanted && !idle_gateways.empty())
	{
		auto* gateway = idle_gateways.back();
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
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		m_build_queue.push_back(unit->getType());
	}
}

/**
 * Remove trained or build unit
 */
void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
	m_last_build_frame = BWAPI::Broodwar->getFrameCount();
}

/**
 * Try to train unit_type
 */
bool ProductionManager::trainUnit(const BWAPI::UnitType& unit_type)
{
	// If we cannot afford unit_type
	if (unit_type.mineralPrice() > getTotalMinerals()) { return false; }
	if (unit_type.gasPrice() > getTotalGas()) { return false; }

	switch (unit_type)
	{
	case BWAPI::UnitTypes::Protoss_Probe:
		{
			auto* depot = Tools::getDepot();
			if (depot) depot->train(unit_type);
			break;
		}
	default:
		{
			std::cout << unit_type << " not supported \n";
			return false;
		}
	}
	return true;
}

bool ProductionManager::trainUnit(const BWAPI::UnitType& unit, BWAPI::Unit depot)
{
	if (unit.mineralPrice() > Global::production().getTotalMinerals()) { return false; }
	if (unit.gasPrice() > Global::production().getTotalGas()) { return false; }

	if (depot && !depot->isTraining())
	{
		depot->train(unit);
		return true;
	}
	return false;
}


// Builds additional supply if needed
void ProductionManager::buildAdditionalSupply()
{
	// Get the amount of supply supply we currently have unused
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	// Only build one supply depot at a time
	if (pendingBuildingsCount(supplyProviderType) > 0) return;

	// If we have a sufficient amount of supply, we don't need to do anything
	if (BWAPI::Broodwar->self()->supplyUsed() + 8 >= Tools::getTotalSupply(true))
	{
		// Otherwise, we are going to build a supply provider
		buildBuilding(supplyProviderType);
	}
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

// Return currently owned minerals, minus the cost of pending build jobs
int ProductionManager::getTotalMinerals()
{
	auto total_minerals = BWAPI::Broodwar->self()->minerals();
	for (auto& build_job : Global::workers().getActiveBuildJobs())
	{
		total_minerals -= build_job.unitType.mineralPrice();
	}
	return total_minerals;
}

// Return currently owned vespene gas, minus the cost of pending build jobs
int ProductionManager::getTotalGas()
{
	auto total_gas = BWAPI::Broodwar->self()->gas();
	for (auto& build_job : Global::workers().getActiveBuildJobs())
	{
		total_gas -= build_job.unitType.gasPrice();
	}
	return total_gas;
}

// Tries to build the desired building type
// TODO: More strategic placement of buildings
bool ProductionManager::buildBuilding(const BWAPI::UnitType type)
{
	// If we have much less gas and minerals than required, it's not worth the wait
	if (getTotalMinerals() < type.mineralPrice() * 0.7) return false;
	if (getTotalGas() < type.gasPrice() * 0.7) return false;

	// Get the type of unit that is required to build the desired building
	const auto builder_type = type.whatBuilds().first;

	// Get a location that we want to build the building next to
	const auto desired_pos = BWAPI::Broodwar->self()->getStartLocation();

	// Ask BWAPI for a building location near the desired position for the type
	const auto max_build_range = 64;
	const auto building_on_creep = type.requiresCreep();
	const auto build_pos = BWAPI::Broodwar->getBuildLocation(type, desired_pos, max_build_range, building_on_creep);

	// Try to build the structure
	auto* builder = Global::workers().getBuilder(builder_type, BWAPI::Position(build_pos));
	if (!builder) { return false; }

	// Assign job to builder unit
	Global::workers().setBuildingWorker(builder, WorkerData::BuildJob{build_pos, type});
	return true;
}
