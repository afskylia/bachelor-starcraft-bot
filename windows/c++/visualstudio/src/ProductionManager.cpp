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
}


void ProductionManager::onFrame()
{
	// Enforce build order
	//tryCompareUnitsAndBuild();
	pollBuildOrder();
	tryBuildOrTrainUnit();

	// Make idle buildings produce units if needed
	activateIdleBuildings();

	// TODO: Instead, automatically add supply depots to build order when about to run out
	buildAdditionalSupply();
}

// Check if anything should be added to the build queue
void ProductionManager::pollBuildOrder()
{
	// Get closest (not null) supply level in build order
	auto supply = BWAPI::Broodwar->self()->supplyUsed() / 2;
	auto test = supply;
	while (!Global::strategy().m_build_order.count(supply))
		supply--;

	// TODO: Check if we have built all the things required by the build order, otherwise enqueue those (using getrequiredunits?)
	/*for (auto [lvl_, _] : Global::strategy().m_build_order)
	{
		if (lvl_ == supply) break;
		if (std::find(enqueued_levels.begin(), enqueued_levels.end(), lvl_) == enqueued_levels.end())
		{
			pushToBuildQueue(lvl_);
			enqueued_levels.push_back(lvl_);
		}
	}*/

	// If we built this last
	if (supply == prev_supply) return;

	// If supply lower than previously, i.e. after an attack by the enemy
	if (supply < prev_supply)
	{
		std::cout << "***** supply < prev_supply ******\n";
		return;
	}

	// Push to build queue and update prev_supply
	pushToBuildQueue(supply);
	prev_supply = supply;
}

// Push unit type at given supply lvl to build queue if not already enqueued
void ProductionManager::pushToBuildQueue(int supply_lvl)
{
	// TODO: Make null proof
	const auto unit_type = Global::strategy().m_build_order[supply_lvl];

	// Make sure this exact supply level has not already been enqueued
	if (std::find(enqueued_levels.begin(), enqueued_levels.end(), supply_lvl) == enqueued_levels.end())
	{
		// Push to build queue and save in enqueued levels for future checks
		pushToBuildQueue(unit_type);
		enqueued_levels.push_back(supply_lvl);
	}
}

// Push given unit type to back of build queue
void ProductionManager::pushToBuildQueue(BWAPI::UnitType unit_type)
{
	m_build_queue_.push_back(unit_type);
}


void ProductionManager::tryBuildOrTrainUnit()
{
	if (m_build_queue_.empty()) return;

	// Access oldest element and remove it
	auto unit_type = m_build_queue_.front();

	// Try to build or train unit, remove from queue upon success
	if (unit_type.isBuilding() && buildBuilding(unit_type) || trainUnit(unit_type)) m_build_queue_.pop_front();
}

bool ProductionManager::addToBuildQueue(const BWAPI::UnitType& unit_type)
{
	// TODO this is a hotfix
	if (unit_type == BWAPI::Broodwar->self()->getRace().getWorker()) return true;

	if (std::find(m_build_queue_.begin(), m_build_queue_.end(), unit_type) == m_build_queue_.end())
	{
		// Add unit to build queue and remove from build order
		m_build_queue_.push_back(unit_type);
		printDebugData();

		// TODO debug
		for (auto build_queue : m_build_queue_)
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
	const int supply = (Tools::getTotalUsedSupply(true) / 2);

	// Iterate build order
	for (auto [supply_lvl, unit_type] : Global::strategy().m_build_order)
	{
		// If unit needed to be build, add it to required_units
		if (supply_lvl <= supply)
		{
			required_units[unit_type]++;
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

	for (auto [req_type, req_amount] : required_units)
	{
		auto owned_units = Tools::getUnitsOfType(req_type, false, false);
		auto amount_owned = owned_units.size();

		//if (owned_units.size() < req_amount) continue;

		while (amount_owned < req_amount)
		{
			addToBuildQueue(req_type);
			amount_owned++;
		}
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
	const auto workers_wanted = 50;
	trainUnitInBuilding(worker_type, workers_wanted);

	auto zealot_type = BWAPI::UnitTypes::Protoss_Zealot;
	auto zealots_wanted = 30;
	trainUnitInBuilding(zealot_type, zealots_wanted);
}

void ProductionManager::trainUnitInBuilding(BWAPI::UnitType unit_type, int units_wanted)
{
	auto idle_buildings = Tools::getUnitsOfType(unit_type.whatBuilds().first, true);
	auto owned = Tools::countUnitsOfType(unit_type);

	while (owned <= units_wanted && !idle_buildings.empty())
	{
		auto* idle_building = idle_buildings.back();
		if (!idle_building) return;
		idle_building->train(unit_type);
		idle_buildings.pop_back();
		owned++;
	}
}

void ProductionManager::printDebugData()
{
	std::cout << "\nBUILD QUEUE: [";
	for (auto s : m_build_queue_)
	{
		std::cout << s << ", ";
	}
	std::cout << "]\n";

	std::cout << "BUILD ORDER: [";
	for (auto [fst, snd] : Global::strategy().m_build_order)
	{
		if (snd == BWAPI::UnitTypes::Protoss_Probe) continue;
		std::cout << "[" << fst << ", " << snd.getName() << "], ";
	}
	std::cout << "]\n\n";
}


/**
 * Update build queue with destroyed units
 */
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getPlayer() == BWAPI::Broodwar->self())
	{
		m_build_queue_.push_back(unit->getType());
	}
}

/**
 * Remove trained or build unit
 */
void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
	m_last_build_frame_ = BWAPI::Broodwar->getFrameCount();
}

/**
 * Try to train unit_type
 */
bool ProductionManager::trainUnit(const BWAPI::UnitType& unit_type)
{
	// If we cannot afford unit_type
	if (unit_type.mineralPrice() > getTotalMinerals()) { return false; }
	if (unit_type.gasPrice() > getTotalGas()) { return false; }


	for (auto [req_type, req_num] : unit_type.requiredUnits())
	{
		auto units = Tools::getUnitsOfType(req_type); //idle=true
		if (units.size() < req_num)
		{
			std::cout << "Need " << req_num << " " << req_type << " to train " << unit_type << "\n";
			return false;
		}
	}

	auto builder_type = unit_type.whatBuilds().first;
	auto builder_unit = Tools::getUnitOfType(builder_type);

	if (builder_unit) return builder_unit->train(unit_type);

	return false;
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
