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
	if (prev_supply > supply)
	{
		if (Global::strategy().m_build_order.count(supply))
		{
			std::cout << "sup\n";
			pushToBuildQueue(supply);
			prev_supply = supply;
		}
		std::cout << "lvl > supply\n";
	}

	// From prev_supply up to current supply, enqueue missing units
	auto lvl = prev_supply + 1;
	while (lvl <= supply)
	{
		if (Global::strategy().m_build_order.count(lvl))
			pushToBuildQueue(lvl);
		lvl++;
	}

	prev_supply = supply;
}

// Push unit type at given supply lvl to build queue if not already enqueued
bool ProductionManager::pushToBuildQueue(int supply_lvl)
{
	// TODO: Make null proof
	const auto unit_type = Global::strategy().m_build_order[supply_lvl];

	// TODO enqueue requirements if we don't already have them

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
			for (auto i = 0; i < number_needed; i++)
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
		{
			std::cout << "Popping another " << unit << "\n";
			m_build_queue_keep_building_.pop_front();
		}
	}
	else
	{
		const auto& unit_type = m_build_queue_.front();

		// Try to build or train unit, remove from queue upon success
		if ((unit_type.isBuilding() && buildBuilding(unit_type)) || trainUnit(unit_type))
		{
			std::cout << "Popping " << unit_type << " from build queue\n";
			m_build_queue_.pop_front();
		}
		else
		{
			auto a = "bai\n";
		}
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
	auto num_workers = Global::workers().m_workerData.getWorkers(WorkerData::Minerals).size();
	auto max_workers = Global::workers().max_workers;
	if (num_workers < max_workers)
		trainUnitInBuilding(worker_type, 1);

	// TODO this seems bugged, only zealots are built

	buildAttackUnits();
	/*auto zealot_type = BWAPI::UnitTypes::Protoss_Zealot;
	auto zealots_wanted = 30;
	trainUnitInBuilding(zealot_type, zealots_wanted);
	trainUnitInBuilding(BWAPI::UnitTypes::Protoss_Dragoon, 30);*/
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

bool ProductionManager::buildBuilding(BWAPI::UnitType type)
{
	const auto* area = Global::map().expos.front();
	if (type == BWAPI::UnitTypes::Protoss_Nexus || type == BWAPI::UnitTypes::Protoss_Photon_Cannon)
		area = Global::map().expos.back();

	return buildBuilding(type, area);
}

// Tries to build the desired building type
// TODO: More strategic placement of buildings
bool ProductionManager::buildBuilding(const BWAPI::UnitType type, const BWEM::Area* area)
{
	// If we have much less gas and minerals than required, it's not worth the wait
	if (getTotalMinerals() < type.mineralPrice() * 0.9) return false;
	if (getTotalGas() < type.gasPrice() * 0.9) return false;

	// Get the type of unit that is required to build the desired building
	const auto builder_type = type.whatBuilds().first;

	// Get a location that we want to build the building next to
	auto desired_pos = BWAPI::TilePosition(area->Bases().front().Center());

	// Ask BWAPI for a building location near the desired position for the type
	const auto building_on_creep = type.requiresCreep();
	const auto build_pos = BWAPI::Broodwar->getBuildLocation(type, desired_pos, 100, building_on_creep);

	if (!BWAPI::Broodwar->canBuildHere(BWAPI::TilePosition(build_pos), type))
	{
		if (type.requiresPsi())
		{
			std::cout << "we need pylons for this one fam\n";
			//auto pylons = Tools::getUnitsOfType(BWAPI::UnitTypes::Protoss_Pylon);
		}
		std::cout << type << " NOT BUILDABLE\n";
		return false;
	}

	// Try to build the structure
	auto* builder = Global::workers().getBuilder(builder_type, BWAPI::Position(build_pos));
	if (!builder) { return false; }

	// Assign job to builder unit
	Global::workers().setBuildingWorker(builder, WorkerData::BuildJob{build_pos, type});
	return true;
}

void ProductionManager::buildAttackUnits()
{
	auto units = Global::combat().m_attack_units;
	for (auto [percentage_of_units_needed, unit_type] : m_build_order_data.attack_unit_list)
	{
		double owned = Tools::countUnitsOfType(unit_type);
		if (units.empty())
		{
			trainUnitInBuilding(unit_type, owned + 5);
			break;
		}
		double units_size = units.size();
		double percentage_owned = owned / units_size;
		// Should we train more units if percentage is not met
		if (percentage_of_units_needed > percentage_owned)
		{
			trainUnitInBuilding(unit_type, owned + 1);
		}
	}
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
	if (unit->getPlayer() == BWAPI::Broodwar->self() && unit->getType().isBuilding())
	{
		std::cout << "Re-adding destroyed " << unit->getType() << " to build queue\n";
		m_build_queue_.push_front(unit->getType());
	}
}

void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
}


// Trains a unit in a building
void ProductionManager::trainUnitInBuilding(BWAPI::UnitType unit_type, int units_wanted)
{
	auto idle_buildings = Tools::getUnitsOfType(unit_type.whatBuilds().first, true);
	auto owned = Tools::countUnitsOfType(unit_type);

	while (!idle_buildings.empty()) //&& owned <= units_wanted
	{
		auto* idle_building = idle_buildings.back();
		if (!idle_building) return;
		if (idle_building->train(unit_type)) return;
		idle_buildings.pop_back();
	}
}

const BWEM::Area* ProductionManager::createNewExpo()
{
	const BWEM::Area* new_area = nullptr;
	auto dist = DBL_MAX;
	auto expos = Global::map().expos;
	const auto* prev = expos.back();

	for (const BWEM::Area& area : Global::map().map.Areas())
	{
		if (!area.AccessibleFrom(Global::map().expos.front()))continue;
		if (area.Bases().empty() || area.Minerals().empty()) continue;

		// Check if we already expanded here
		if (std::find(expos.begin(), expos.end(), &area) != expos.end()) continue;

		// Check if this is an enemy base // TODO what if we don't know yet?
		if (std::find(Global::information().enemy_areas.begin(), Global::information().enemy_areas.end(), &area) !=
			Global::information().enemy_areas.end())
			continue;

		const auto _dist = prev->Bases()[0].Center().getDistance(area.Bases()[0].Center());
		if (_dist < dist)
		{
			new_area = &area;
			dist = _dist;
		}
	}

	Global::map().expos.push_back(new_area);
	m_build_queue_.push_front(BWAPI::UnitTypes::Protoss_Nexus);
	// TODO: Cannons require a pylon, so it will glitch out if the pylon hasn't been built yet... find a way to fix this bug
	//m_build_queue_.push_front(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	m_build_queue_.push_front(BWAPI::UnitTypes::Protoss_Pylon);

	return new_area;
}
