#include "ProductionManager.h"

#include "Global.h"
#include "Tools.h"
#include "WorkerManager.h"

using namespace MiraBot;

ProductionManager::ProductionManager() = default;


void ProductionManager::onFrame()
{
	// Add units to build queue if possible
	pollBuildOrder();

	// Try to build unit from build queue
	tryBuildOrTrainUnit();

	// Build additional if needed
	buildAdditionalSupply();

	// Checks all buildings if they can be upgraded.
	checkIfUpgradesAreAvailable();

	// Make idle buildings produce units if possible
	activateIdleBuildings();
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
			pushToBuildQueue(supply);
			prev_supply = supply;
		}

		return;

		/*prev_supply = supply;
		return;*/
	}

	// From prev_supply up to current supply, enqueue missing units
	auto lvl = prev_supply + 1;
	while (lvl <= supply)
	{
		if (Global::strategy().m_build_order.count(lvl))
		{
			if (!pushToBuildQueue(lvl))
			{
				prev_supply = lvl;
				return;
			}
		}
		lvl++;
	}

	prev_supply = supply;
}

// Push unit type at given supply lvl to build queue if not already enqueued
bool ProductionManager::pushToBuildQueue(int supply_lvl)
{
	const auto unit_type = Global::strategy().m_build_order[supply_lvl];
	const auto unit = unit_type.first;
	const auto number_needed = unit_type.second;

	// Make sure this exact supply level has not already been enqueued
	if (std::find(enqueued_levels.begin(), enqueued_levels.end(), supply_lvl) == enqueued_levels.end())
	{
		// Push to build queue and save in enqueued levels for future checks
		pushToBuildQueue(unit);
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

bool ProductionManager::pushToBuildQueue(BWAPI::UnitType unit_type)
{
	// Check if requirements are met - push them otherwise
	for (auto [req_type, req_num] : unit_type.requiredUnits())
	{
		if (req_type == BWAPI::UnitTypes::None) continue;
		auto is_active_buildjob = false;

		for (auto b : Global::workers().getActiveBuildJobs())
		{
			if (b.unitType == req_type) is_active_buildjob = true;
			break;
		}

		if (std::count(m_build_queue_.begin(), m_build_queue_.end(), req_type) ||
			Tools::countUnitsOfType(req_type, false) || is_active_buildjob)
			continue;

		auto flag = true;
		for (auto u : BWAPI::Broodwar->getAllUnits())
		{
			if (u->getType() == req_type && (u->getPlayer() == BWAPI::Broodwar->self() || u->getPlayer()->isNeutral()))
			{
				flag = true;
				break;
			}
		}
		if (!flag) continue;

		std::cout << "Enqueueing missing requirement for " << unit_type << ", " << req_type << "\n";
		pushToBuildQueue(req_type);
		//// Count number of enqueued units + number of completed units
		//auto req_count = Tools::countUnitsOfType(req_type);
		//for (auto type : m_build_queue_)
		//	if (type == req_type) req_count++;

		//// Enqueue missing 
		//while (req_count < req_num)
		//{
		//	std::cout << "Enqueueing missing requirement for " << unit_type << ", " << req_type << "\n";
		//	pushToBuildQueue(req_type);
		//	req_count++;
		//}
	}

	m_build_queue_.push_back(unit_type);
	return true;
}


// Try to build/train the oldest element of the build queue
void ProductionManager::tryBuildOrTrainUnit()
{
	// Try to build or train unit, remove from queue upon success
	if (!m_build_queue_.empty())
	{
		const auto& unit_type = m_build_queue_.front();
		if (unit_type == BWAPI::UnitTypes::Protoss_Nexus)
		{
			int i = 0;
		}
		if (unit_type.isBuilding() && buildBuilding(unit_type) || trainUnit(unit_type))
		{
			m_build_queue_.pop_front();
			return;
		}

		// Nexuses take 400 so we'll allow some specific types to be built before it
		if (unit_type == BWAPI::UnitTypes::Protoss_Nexus && m_build_queue_.size() >= 2)
		{
			const auto& second_unit_type = m_build_queue_.at(1);
			std::vector<BWAPI::UnitType> allowed_types = {
				BWAPI::UnitTypes::Protoss_Cybernetics_Core, BWAPI::UnitTypes::Protoss_Forge
			};

			if (std::count(allowed_types.begin(), allowed_types.end(), second_unit_type))
			{
				if (second_unit_type.isBuilding() && buildBuilding(second_unit_type) || trainUnit(second_unit_type))
				{
					m_build_queue_.erase(m_build_queue_.begin() + 1);
					return;
				}
			}
		}
	}


	// Build repeated units
	if (!m_build_queue_keep_building_.empty())
	{
		const auto& unit = m_build_queue_keep_building_.front();
		if (unit.isBuilding() && buildBuilding(unit) || trainUnit(unit))
		{
			m_build_queue_keep_building_.pop_front();
		}
	}
}


void ProductionManager::activateIdleBuildings()
{
	/**
	 * TODO: Maybe make a class (InformationManager?) that stores relevant information
	 * Such as all the races and their unit types, how many of each type/job we want etc.
	 * Then this function can iterate the list of needed units in a smart way.
	 *
	 * TODO: Only train units if we can afford it
	 * (I.e. it won't make us too poor to afford higher priority units/upgrades.)
	 */

	/**
	 * Go through idle Gateways and activate them.
	 */

	// Fill map with amount of our different attack units
	std::map<BWAPI::UnitType, int> attack_unit_distribution = {};
	for (auto* unit : Global::combat().m_attack_units) attack_unit_distribution[unit->getType()]++;

	// Activate each idle gateway
	auto idle_gateways = Tools::getUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway, true);
	auto available_minerals = getTotalMinerals(true);
	auto available_gas = getTotalGas(true);

	if (!idle_gateways.empty())
	{
		for (auto* gateway : idle_gateways)
		{
			const auto attack_type = getUnitToTrain(attack_unit_distribution, available_minerals, available_gas);

			// If we don't have enough resources to train attack units right now
			if (attack_type == BWAPI::UnitTypes::None) break;

			// Try to train the attack unit
			if (gateway->train(attack_type))
			{
				//BWAPI::Broodwar->printf("Training %s", attack_type.c_str());
				attack_unit_distribution[attack_type]++;
				available_minerals -= attack_type.mineralPrice();
				available_gas -= attack_type.gasPrice();
			}
		}
	}

	// Go through idle Nexuses and activate them
	const auto worker_type = BWAPI::Broodwar->self()->getRace().getWorker();
	const auto num_workers = Global::workers().m_workerData.getWorkers(WorkerData::Minerals).size();
	const auto max_workers = Global::workers().max_workers;
	if (num_workers < max_workers)
		trainUnitInBuilding(worker_type, max_workers);
}


// Try to train unit_type
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
	// Default to placing buildings in the main base
	const auto* area = Global::map().expos.front();

	// Nexuses are built in the last expo that was created
	if (type == BWAPI::UnitTypes::Protoss_Nexus)
		area = Global::map().expos.back();

	// Evenly distributes the areas where pylons are built
	if (type == BWAPI::UnitTypes::Protoss_Pylon)
	{
		const BWEM::Area* fewest_pylons = nullptr;
		auto lowest_count = INT_MAX;
		for (auto area : Global::map().expos)
		{
			auto count = 0;
			for (auto* u : BWAPI::Broodwar->getUnitsInRadius(area->Bases()[0].Center(), 550))
				if (u->getType() == type && Global::map().map.GetNearestArea(u->getTilePosition()) == area) count++;

			if (count < lowest_count)
			{
				fewest_pylons = area;
				lowest_count = count;
			}
		}
		area = fewest_pylons;
	}

	// Evenly distributes the areas where pylons are built
	if (type == BWAPI::UnitTypes::Protoss_Photon_Cannon)
	{
		const BWEM::Area* fewest_pylons = nullptr;
		auto lowest_count = INT_MAX;
		for (auto area : Global::map().expos)
		{
			auto count = 0;
			for (auto* u : BWAPI::Broodwar->getUnitsInRadius(area->Bases()[0].Center(), 550))
				if (u->getType() == type && Global::map().map.GetNearestArea(u->getTilePosition()) == area) count++;

			if (count < lowest_count)
			{
				fewest_pylons = area;
				lowest_count = count;
			}
		}
		area = fewest_pylons;
	}

	return buildBuilding(type, area);
}

// Tries to build the desired building type
bool ProductionManager::buildBuilding(const BWAPI::UnitType type, const BWEM::Area* area)
{
	// Check that all requirements are met for this unit type
	for (const auto req : type.requiredUnits())
		if (!Tools::countUnitsOfType(req.first)) return false;

	// If we have much less gas and minerals than required, it's not worth the wait
	if (getTotalMinerals() < type.mineralPrice() * 0.9) return false;
	if (getTotalGas() < type.gasPrice() * 0.9) return false;

	// Get the type of unit that is required to build the desired building
	const auto builder_type = type.whatBuilds().first;

	// Get a location that we want to build the building near
	auto desired_pos = BWAPI::TilePosition(area->Bases().front().Center());

	// If unit is e.g. a cannon, place near chokepoint closest to enemy starting location
	if (type.canAttack()) desired_pos = BWAPI::TilePosition(Global::map().getClosestCP(area)->Center());

	// Get building location near the desired position for the type, using BuildingPlacer from bwsal
	auto build_pos = m_building_placer_.getBuildLocationNear(desired_pos, type);
	if (type == BWAPI::UnitTypes::Protoss_Assimilator)
	{
		auto existing_refineries = Tools::getUnitsOfType(BWAPI::UnitTypes::Protoss_Assimilator, false, false);
		BWAPI::Unit closest_geyser = nullptr;
		auto dist = DBL_MAX;
		for (const auto& g : Global::map().map.Geysers())
		{
			auto d = g->Unit()->getPosition().getDistance(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()));
			if (d < dist)
			{
				auto noflag = false;
				for (auto ref : existing_refineries)
				{
					if (ref->getPosition().getDistance(g->Unit()->getPosition()) < 100)
					{
						noflag = true;
						break;
					}
				}
				if (noflag) continue;

				closest_geyser = g->Unit();
				dist = d;
			}
		}
		build_pos = m_building_placer_.getBuildLocationNear(BWAPI::TilePosition(closest_geyser->getPosition()), type);
	}
	if (!m_building_placer_.canBuildHereWithSpace(build_pos, type))
	{
		// Pick any valid build location if the found one doesn't work
		build_pos = m_building_placer_.getBuildLocation(type);
		if (type == BWAPI::UnitTypes::Protoss_Assimilator) return false;
	}


	// Return false if this can't be built for whatever reason
	if (!BWAPI::Broodwar->canBuildHere(BWAPI::TilePosition(build_pos), type))
		return false;

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
		int owned = Tools::countUnitsOfType(unit_type);
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

BWAPI::UnitType ProductionManager::getUnitToTrain(std::map<BWAPI::UnitType, int> distribution, int minerals, int gas)
{
	auto type_to_train = BWAPI::UnitTypes::None;

	// If we don't have any attack units, build a zealot
	if (distribution.empty())
	{
		if (minerals >= 100) type_to_train = BWAPI::UnitTypes::Protoss_Zealot;
		return type_to_train;
	}

	if (minerals < 100) return type_to_train;

	type_to_train = BWAPI::UnitTypes::Protoss_Zealot;

	// Compute total amount of attack units in unit distribution map
	auto total_attack_units = 0;
	for (auto [type, amount] : distribution) total_attack_units += amount;

	// Get the attack unit type we have the least of (with regards to the goal percentage)
	auto least_percentage = FLT_MAX;
	for (auto [percentage_of_units_needed, unit_type] : m_build_order_data.attack_unit_list)
	{
		// Check if unit requirements are met, e.g. Gateways for Dragoons
		auto requirements_met = true;
		for (auto req : unit_type.requiredUnits())
		{
			if (Tools::countUnitsOfType(req.first) < req.second)
			{
				requirements_met = false;
				break;
			}
		}

		// Check if we can afford it
		if (unit_type.mineralPrice() > minerals || unit_type.gasPrice() > gas) requirements_met = false;

		// Continue if requirements for this unit type are not met
		if (!requirements_met) continue;

		// Compute percentage of units owned and compare with current least perecentage owned
		auto percentage_owned = static_cast<float>(distribution[unit_type]) / static_cast<float>(total_attack_units);
		if (percentage_owned < least_percentage)
		{
			type_to_train = unit_type;
			least_percentage = percentage_owned;
		}
	}

	return type_to_train;
}

// Builds additional supply if needed
void ProductionManager::buildAdditionalSupply()
{
	// Get the amount of supply supply we currently have unused
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	// Only build one supply depot at a time
	//if (pendingBuildingsCount(supplyProviderType) > 0) return;

	// If we have a sufficient amount of supply, we don't need to do anything
	if (BWAPI::Broodwar->self()->supplyUsed() + 20 >= Tools::getTotalSupply(true))
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
int ProductionManager::getTotalMinerals(bool excludingFrontOfBuildQueue)
{
	auto total_minerals = BWAPI::Broodwar->self()->minerals();
	for (auto& build_job : Global::workers().getActiveBuildJobs())
	{
		total_minerals -= build_job.unitType.mineralPrice();
	}

	if (excludingFrontOfBuildQueue && !m_build_queue_.empty())
	{
		total_minerals -= m_build_queue_.front().mineralPrice();
	}

	return total_minerals;
}

// Return currently owned vespene gas, minus the cost of pending build jobs
int ProductionManager::getTotalGas(bool excludingFrontOfBuildQueue)
{
	auto total_gas = BWAPI::Broodwar->self()->gas();
	for (auto& build_job : Global::workers().getActiveBuildJobs())
	{
		total_gas -= build_job.unitType.gasPrice();
	}

	if (excludingFrontOfBuildQueue && !m_build_queue_.empty())
	{
		total_gas -= m_build_queue_.front().gasPrice();
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
	if (unit->getType() == BWAPI::UnitTypes::Resource_Vespene_Geyser)
	{
		std::cout << "GAS DEPLETED\n";
		m_build_queue_.push_front(BWAPI::Broodwar->self()->getRace().getRefinery());
		return;
	}

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

	while (!idle_buildings.empty() && owned <= units_wanted) //&& owned <= units_wanted
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

	// Iterate over all areas in the map
	for (const BWEM::Area& area : Global::map().map.Areas())
	{
		// Don't use area if unreachable from the main base, or if it has no bases or minerals
		if (!area.AccessibleFrom(Global::map().expos.front()))continue;
		if (area.Bases().empty() || area.Minerals().empty()) continue;

		// Check if we already expanded to here
		if (std::find(expos.begin(), expos.end(), &area) != expos.end()) continue;

		// Check if this is an enemy base
		if (std::find(Global::information().enemy_areas.begin(), Global::information().enemy_areas.end(), &area) !=
			Global::information().enemy_areas.end())
			continue;

		// Compute distance the latest base expansion
		const auto _dist = Global::map().map.GetPath(prev->Bases()[0].Center(),
		                                             area.Bases()[0].Center()).size();
		if (_dist < dist)
		{
			new_area = &area;
			dist = _dist;
		}
	}

	Global::map().expos.push_back(new_area);
	m_build_queue_keep_building_.push_front(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	m_build_queue_keep_building_.push_front(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	m_build_queue_keep_building_.push_front(BWAPI::UnitTypes::Protoss_Photon_Cannon);
	m_build_queue_.push_front(BWAPI::UnitTypes::Protoss_Nexus);
	m_build_queue_.push_front(BWAPI::UnitTypes::Protoss_Pylon);

	return new_area;
}
