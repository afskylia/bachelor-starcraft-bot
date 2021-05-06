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
	Global::Workers().test();
	//Global::Map().
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

	const auto unitType = BWAPI::UnitTypes::Protoss_Gateway;
	if (BWAPI::Broodwar->self()->minerals() < unitType.mineralPrice()) { return; }

	if (countBuildings(unitType,true) < 4)
	{
		auto startedBuilding = Global::Workers().buildBuilding(unitType);
		if (startedBuilding)std::cout << "Assigned build job: " << unitType.getName() << "\n";

	}
}

void ProductionManager::buildAttackUnits()
{
	const auto unitType = BWAPI::UnitTypes::Protoss_Zealot;
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
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	// Only build one supply depot at a time
	if (pendingBuildingsCount(supplyProviderType) > 0) return;

	// If we have a sufficient amount of supply, we don't need to do anything
	if (BWAPI::Broodwar->self()->supplyUsed() + 8 >= Tools::GetTotalSupply(true))
	{
		//if (unusedSupply >= 3) { return; }
		//BWAPI::UnitTypes::Protoss_Zealot.supplyRequired()

		// Otherwise, we are going to build a supply provider
		//if (BWAPI::Broodwar->self()->minerals() < supplyProviderType.mineralPrice()) { return; }
		const auto startedBuilding = Global::Workers().buildBuilding(supplyProviderType);
		if (startedBuilding)
		{
			BWAPI::Broodwar->printf("Started Building %s", supplyProviderType.getName().c_str());
		}
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
