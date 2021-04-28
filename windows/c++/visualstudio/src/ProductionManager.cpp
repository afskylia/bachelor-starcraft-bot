#include "ProductionManager.h"
#include "BuildOrderData.h"

#include "Tools.h"


/*
 * Everything production related
 */
ProductionManager::ProductionManager()
{
	
}

void ProductionManager::onFrame()
{
	// Build more supply if we are going to run out soon
	/*buildAdditionalSupply();
	buildGateway();
	buildAttackUnits();*/
	// buildFromBuildOrder();
}

void ProductionManager::onStart()
{
	buildFromBuildOrder();
}

void ProductionManager::onUnitComplete(BWAPI::Unit unit)
{
	buildFromBuildOrder();
}

void ProductionManager::buildFromBuildOrder()
{
	BWAPI::UnitType unit = BWAPI::UnitTypes::None;
	const int supply = BWAPI::Broodwar->self()->supplyUsed() / 2;
	//for (auto itr = build.steps.begin(); itr != build.steps.end(); ++itr) {
	//	std::cout << itr->first
	//		<< '\t' << itr->second << '\n';
	//}
	std::cout << unit;
	std::map<int, BWAPI::UnitType>::iterator it = build.steps.find(supply);
	if (it == build.steps.end())
		return;
	unit = it->second;

	std::cout << unit;
	if (unit.isBuilding())
	{
		if (Tools::BuildBuilding(unit)) build.steps.erase(it);
		
	} else
	{
		if (trainUnit(unit)) build.steps.erase(it);
	}
}

void ProductionManager::buildGateway()
{
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

void ProductionManager::buildAttackUnits()
{
	const BWAPI::UnitType unitType = BWAPI::UnitTypes::Protoss_Zealot;
	auto gateways = Tools::GetUnitsOfType(BWAPI::UnitTypes::Protoss_Gateway);
	for (auto* gateway : gateways)
	{
		if (gateway && !gateway->isTraining()) { gateway->train(unitType); }
	}
}

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
		if (myDepot && !myDepot->isTraining()) { myDepot->train(unit); }
		}
	default: std::cout << "unit not supported \n";
	}
	return true;
}