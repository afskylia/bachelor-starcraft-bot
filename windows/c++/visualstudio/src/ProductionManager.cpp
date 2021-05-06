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
		m_build_order[i] = BWAPI::UnitTypes::Protoss_Probe;
	}
	m_build_order[4] = BWAPI::UnitTypes::Protoss_Nexus;
	m_build_order[8] = BWAPI::UnitTypes::Protoss_Pylon;
	m_build_order[10] = BWAPI::UnitTypes::Protoss_Gateway;
	m_build_order[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	m_build_order[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
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
		bool builded = buildBuilding(unit_type);
		if (!builded) m_last_build_frame = BWAPI::Broodwar->getFrameCount();
	}
	else
	{
		bool builded = trainUnit(unit_type);
		if (!builded) m_last_build_frame = BWAPI::Broodwar->getFrameCount();
	}
}

bool ProductionManager::addToBuildQueue(const BWAPI::UnitType& unit_type)
{
	if (std::find(m_build_queue.begin(), m_build_queue.end(), unit_type) == m_build_queue.end()) {
		// Add unit to build queue and remove from build order
		m_build_queue.push_back(unit_type);

		// TODO debug
		for (auto build_queue : m_build_queue)
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
	const int supply = (Tools::GetTotalUsedSupply(true) / 2);

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
		if (it == all_units.end()) {
			addToBuildQueue(required_unit.first);
			break;
		}
		if (required_unit.second > it->second)
			addToBuildQueue(required_unit.first);
	}
	tryBuildOrTrainUnit();
}

/**
 * Try to compare units with required units and build difference
 */
void ProductionManager::tryCompareUnitsAndBuild()
{
	const int frame_count = BWAPI::Broodwar->getFrameCount();
	if (frame_count == m_last_build_frame + 20) {
		compareUnitsAndBuild();
	}
}

void ProductionManager::onFrame()
{
	tryCompareUnitsAndBuild();
	// TODO: train workers in idle buildings
	// TODO: late game, train attack units in idle buildings
	// TODO: build supply depots when about to run out
}

/**
 * Update build queue with destroyed units
 */
void ProductionManager::onUnitDestroy(BWAPI::Unit unit)
{
	m_build_queue.push_back(unit->getType());
}

/**
 * Remove trained or build unit
 */
void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
	m_last_build_frame = BWAPI::Broodwar->getFrameCount();
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
	default: {
		std::cout << unit << " not supported \n";
		return false;}
	}
	return true;
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
	auto buildJobs = Global::Workers().getActiveBuildJobs();
	return std::size(buildJobs);
}

// Returns number of pending buildings of given type (build job assigned but not yet built)
int ProductionManager::pendingBuildingsCount(BWAPI::UnitType type)
{
	auto buildJobs = Global::Workers().getActiveBuildJobs(type);
	return std::size(buildJobs);
}

// Return currently owned minerals, INCLUDING cost of pending build jobs
int ProductionManager::getTotalMinerals()
{
	auto totalMinerals = BWAPI::Broodwar->self()->minerals();

	for (auto& buildJob : Global::Workers().getActiveBuildJobs())
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
	if (Global::Production().getTotalMinerals() < type.mineralPrice() * 0.7) return false;
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
	auto* builder = Global::Workers().getBuilder(builderType, BWAPI::Position(buildPos));
	if (!builder) { return false; }

	Global::Workers().setBuildingWorker(builder, WorkerData::BuildJob{ buildPos,type });
	return true;
}

