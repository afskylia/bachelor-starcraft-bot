#include "ProductionManager.h"

#include "Global.h"
#include "Tools.h"
#include "WorkerManager.h"

using namespace MiraBot;

ProductionManager::ProductionManager() = default;


void ProductionManager::onFrame()
{
	// Add units to build queue
	pollBuildOrder();

	// Build unit from build queue
	tryBuildOrTrainUnit();

	// Make idle buildings produce units if needed
	/*TODO: when training something in trybuildortrainunit, does the building then become
	 * non-idle in the same frame? if not that is an issue because it will be overridden!*/
	activateIdleBuildings();

	// TODO: Instead, automatically add supply depots to build order when about to run out?
	buildAdditionalSupply();

	// Checks all buildings if they can be upgraded.
	checkIfUpgradesAreAvailable();
}

// Check if anything should be added to the build queue
void ProductionManager::pollBuildOrder()
{
	// Get closest (not null) supply level in build order
	auto supply = BWAPI::Broodwar->self()->supplyUsed() / 2;
	while (!Global::strategy().m_build_order.count(supply))
		supply--;

	if (supply == prev_supply) return;
	if (prev_supply > supply) std::cout << "lvl > supply\n";

	// From prev_supply up to current supply, enqueue missing units
	auto lvl = prev_supply + 1;
	while (lvl <= supply)
	{
		if (Global::strategy().m_build_order.count(lvl))
			pushToBuildQueue(lvl);
		lvl++;
	}

	prev_supply = supply;


	//// TODO: Check if we have built all the things required by the build order, otherwise enqueue those (using getrequiredunits?)
	///*for (auto [lvl_, _] : Global::strategy().m_build_order)
	//{
	//	if (lvl_ == supply) break;
	//	if (std::find(enqueued_levels.begin(), enqueued_levels.end(), lvl_) == enqueued_levels.end())
	//	{
	//		pushToBuildQueue(lvl_);
	//		enqueued_levels.push_back(lvl_);
	//	}
	//}*/

	//// If we built this last
	//if (supply == prev_supply) return;

	//// If supply lower than previously, i.e. after an attack by the enemy
	//if (supply < prev_supply)
	//{
	//	// TODO figure out what to do there
	//	std::cout << "***** supply < prev_supply ******\n";
	//	return;
	//}

	//// Push to build queue and update prev_supply
	//pushToBuildQueue(supply);
	//prev_supply = supply;
}

// Push unit type at given supply lvl to build queue if not already enqueued
bool ProductionManager::pushToBuildQueue(int supply_lvl)
{
	// TODO: Make null proof
	const auto unit_type = Global::strategy().m_build_order[supply_lvl];

	const auto unit = unit_type.first;
	const auto number_needed = unit_type.second;

	// Make sure this exact supply level has not already been enqueued
	if (std::find(enqueued_levels.begin(), enqueued_levels.end(), supply_lvl) == enqueued_levels.end())
	{
		// Push to build queue and save in enqueued levels for future checks
		m_build_queue_.push_back(unit);
		enqueued_levels.push_back(supply_lvl);
		std::cout << "Added " << unit << " to build queue\n";
		if (number_needed > 1)
		{
			for (auto i = 0; i <= number_needed; i++)
			{
				m_build_queue_keep_building_.push_back(unit);
			}
		}
		return true;
	}
	return false;
}


// Try to build/train the oldest element of the build queue
void ProductionManager::tryBuildOrTrainUnit()
{
	if (m_build_queue_.empty())
	{
		if (m_build_queue_keep_building_.empty()) return;

		const auto& unit = m_build_queue_keep_building_.front();
		if (unit.isBuilding() && buildBuilding(unit) || trainUnit(unit))
			m_build_queue_keep_building_.pop_front();
	}
	else
	{
		const auto& unit_type = m_build_queue_.front();

		// Try to build or train unit, remove from queue upon success
		if (unit_type.isBuilding() && buildBuilding(unit_type) || trainUnit(unit_type))
			m_build_queue_.pop_front();
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

	// TODO this seems bugged, only zealots are built
	auto zealot_type = BWAPI::UnitTypes::Protoss_Zealot;
	auto zealots_wanted = 30;
	trainUnitInBuilding(zealot_type, zealots_wanted);
	trainUnitInBuilding(BWAPI::UnitTypes::Protoss_Dragoon, 30);
}


// Try to train unit_type
// TODO: Optional "depot/building" parameter: the building in which to train unit (or in an overload)
bool ProductionManager::trainUnit(const BWAPI::UnitType& unit_type)
{
	// Return if we cannot afford the unit
	if (unit_type.mineralPrice() > getTotalMinerals()) { return false; }
	if (unit_type.gasPrice() > getTotalGas()) { return false; }

	// Ensure that we have all required units (Protoss_Archon requires 2 of same type!)
	// TODO: Enqueue missing units?
	for (auto [req_type, req_num] : unit_type.requiredUnits())
		if (Tools::getUnitsOfType(req_type).size() < req_num) return false;

	// Find (preferably idle) builder unit
	const auto builder_type = unit_type.whatBuilds().first;
	auto* builder_unit = Tools::getUnitOfType(builder_type, true);
	if (!builder_unit) builder_unit = Tools::getUnitOfType(builder_type);

	// Try to train the unit
	if (builder_unit && builder_unit->train(unit_type))
	{
		std::cout << "Now training " << unit_type << "\n";
		return true;
	}
	return false;
}


// Tries to build the desired building type
// TODO: More strategic placement of buildings
bool ProductionManager::buildBuilding(const BWAPI::UnitType type)
{
	// If we have much less gas and minerals than required, it's not worth the wait
	if (getTotalMinerals() < type.mineralPrice() * 0.9) return false;
	if (getTotalGas() < type.gasPrice() * 0.9) return false;

	// Get the type of unit that is required to build the desired building
	const auto builder_type = type.whatBuilds().first;

	// Get a location that we want to build the building next to
	auto desired_pos = BWAPI::Broodwar->self()->getStartLocation();

	if (type == BWAPI::UnitTypes::Protoss_Photon_Cannon)
	{
		const auto* area = Global::map().main_area;
		auto chokepoints = area->ChokePoints();
		for (const auto* cp : chokepoints)
		{
			auto cp_center = BWAPI::Position(cp->Center());
			auto units_in_radius = BWAPI::Broodwar->getUnitsInRadius(cp_center, 64, BWAPI::Filter::IsBuilding);
			auto cannon_count = 0;
			for (auto u : units_in_radius)
			{
				if (u->getType() == type)
					cannon_count++;
			}

			// Use this CP if less than 2 cannons nearby
			if (cannon_count < 2)
			{
				desired_pos = BWAPI::TilePosition(cp_center);
				break;
			}
		}

		if (desired_pos == BWAPI::Broodwar->self()->getStartLocation())
		{
			std::cout << "All chokepoints in main base are already guarded\n";
			return true;
		}
	}

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

// Builds additional supply if needed
void ProductionManager::buildAdditionalSupply()
{
	// Get the amount of supply supply we currently have unused
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	// Only build one supply depot at a time
	if (pendingBuildingsCount(supplyProviderType) > 0) return;

	// If we have a sufficient amount of supply, we don't need to do anything TODO Still builds past supply 200, but we might need psi
	/*if (BWAPI::Broodwar->self()->supplyTotal() != 200 && BWAPI::Broodwar->self()->supplyUsed() * 1.3 >=
		Tools::getTotalSupply(true))*/
	if (BWAPI::Broodwar->self()->supplyUsed() + 10 >= Tools::getTotalSupply(true))
	{
		// Otherwise, we are going to build a supply provider
		buildBuilding(supplyProviderType);
	}
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

// Prints the current build queue and build order to the console (type "production")
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
		if (snd.first == BWAPI::UnitTypes::Protoss_Probe) continue;
		std::cout << "[" << fst << ", " << snd.first.getName() << "], ";
	}
	std::cout << "]\n\n";
}

/// <summary>
/// Go through all buildings to see if we can upgrade something
/// </summary>
void ProductionManager::checkIfUpgradesAreAvailable()
{
	auto buildings = Global::information().main_base->getUnitsInRadius(
		9999, BWAPI::Filter::IsBuilding && BWAPI::Filter::IsOwned);

	// No buildings
	if (buildings.empty()) return;

	auto& upgrades = BWAPI::UpgradeTypes::allUpgradeTypes();

	for (auto* building : buildings)
	{
		if (!building->canUpgrade() || building->isTraining() || building->isUpgrading()) continue;

		for (auto& upgrade : upgrades)
		{
			if (building->canUpgrade(upgrade))
			{
				building->upgrade(upgrade);
				break;
			}
		}
	}
}

// Rebuild destroyed strategic units (units from build order)
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	// TODO: Enqueue only if unit type belongs to a level in enqueued_levels
	if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType().isBuilding())
	{
		m_build_queue_.push_back(unit->getType());
	}
}

void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
}

void ProductionManager::addToBuildQueue(BWAPI::UnitType unit_type)
{
	int id = unit_type.getID();
	m_build_queue_.push_back(unit_type);
}

/**
 * Try to compare units with required units and build difference.
 * This is not enforced on every frame of the game, since that would
 * lead to buildings being enqueued multiple times due to delays.
 */
[[deprecated]]
void ProductionManager::tryCompareUnitsAndBuild()
{
	const int frame_count = BWAPI::Broodwar->getFrameCount();
	if (frame_count == m_last_build_frame_ + 20)
	{
		compareUnitsAndBuild();
	}
}

[[deprecated]]
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

/**
 * Compare units with required units and build difference
 */
[[deprecated]]
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
 * @return map of all built units <number of units, UnitType>
 */
[[deprecated]]
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
[[deprecated]]
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
			required_units[unit_type.first]++;
		}
	}

	return required_units;
}


// Trains a unit in a building
void ProductionManager::trainUnitInBuilding(BWAPI::UnitType unit_type, int units_wanted)
{
	auto idle_buildings = Tools::getUnitsOfType(unit_type.whatBuilds().first, true);
	auto owned = Tools::countUnitsOfType(unit_type);

	while (owned <= units_wanted && !idle_buildings.empty())
	{
		auto* idle_building = idle_buildings.back();
		if (!idle_building) return;
		if (idle_building->train(unit_type)) return;
		idle_buildings.pop_back();
		owned++;
	}
}
