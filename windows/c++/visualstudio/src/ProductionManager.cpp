#include "ProductionManager.h"

#include "Tools.h"
#include "WorkerManager.h"


/*
 * Everything production related
 */
ProductionManager::ProductionManager()
{
}

void ProductionManager::onFrame()
{
	// Build more supply if we are going to run out soon
	buildAdditionalSupply();
	buildGateway();
	buildAttackUnits();
}

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