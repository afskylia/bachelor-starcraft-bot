#include "ProductionManager.h"

#include "Tools.h"
#include "WorkerManager.h"


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
		bool builded = Tools::BuildBuilding(unit_type);
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
			std::cout << build_queue;
		}
		std::cout << "\n";
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
	// Get the amount of supply supply we currently have unused
	//const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();
	//if (unusedSupply > 7) { return; }

	const BWAPI::UnitType unitType = BWAPI::UnitTypes::Protoss_Gateway;
	if (BWAPI::Broodwar->self()->minerals() < unitType.mineralPrice()) { return; }

	if (Tools::CountUnitsOfType(unitType) < 4)
	{
		const bool startedBuilding = Tools::BuildBuilding(unitType);
		if (startedBuilding)
		{
			BWAPI::Broodwar->printf("Started Building %s", unitType.getName().c_str());
		}

	}
}

/**
 *@deprecated use build order
 */
[[deprecated]]
void ProductionManager::buildAttackUnits()
{
	const BWAPI::UnitType unitType = BWAPI::UnitTypes::Protoss_Zealot;
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
//const int unusedSupply = Tools::GetTotalSupply(true) - BWAPI::Broodwar->self()->supplyUsed();

// If we have a sufficient amount of supply, we don't need to do anything
	if (BWAPI::Broodwar->self()->supplyUsed() + 8 >= Tools::GetTotalSupply(true))
	{
		//if (unusedSupply >= 3) { return; }
		//BWAPI::UnitTypes::Protoss_Zealot.supplyRequired()

		// Otherwise, we are going to build a supply provider
		const BWAPI::UnitType supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

		//std::cout << BWAPI::Broodwar->self()->mine
		if (BWAPI::Broodwar->self()->minerals() < supplyProviderType.mineralPrice()) { return; }
		const bool startedBuilding = Tools::BuildBuilding(supplyProviderType);
		if (startedBuilding)
		{
			BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
		}
	}
}