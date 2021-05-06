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
	const auto unitType = BWAPI::UnitTypes::Protoss_Gateway;
	if (countBuildings(unitType, true) < 4)
	{
		auto startedBuilding = buildBuilding(unitType);
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
	const auto supplyProviderType = BWAPI::Broodwar->self()->getRace().getSupplyProvider();

	// Only build one supply depot at a time
	if (pendingBuildingsCount(supplyProviderType) > 0) return;

	// If we have a sufficient amount of supply, we don't need to do anything
	if (BWAPI::Broodwar->self()->supplyUsed() + 8 >= Tools::GetTotalSupply(true))
	{
		// Otherwise, we are going to build a supply provider
		const auto startedBuilding = buildBuilding(supplyProviderType);
		if (startedBuilding) std::cout << "Assigned build job: " << supplyProviderType.getName() << "\n";

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

