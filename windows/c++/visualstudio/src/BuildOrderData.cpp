#include "BuildOrderData.h"

using namespace MiraBot;

BuildOrderData::BuildOrderData()
{
	for (int i = 0; i <= 200; ++i)
	{
		// TODO: Revamp of build order system so it doesn't have probes on empty levels
		clean_build_order[i] = BWAPI::UnitTypes::Protoss_Probe;
	}
	clean_build_order[4] = BWAPI::UnitTypes::Protoss_Nexus;

	initBuildOrders();
}

void BuildOrderData::initBuildOrders()
{
	initStarterBuildOrder();
	initProtossVTerranBuildOrder();
	initProtossVProtossBuildOrder();
	initProtossVZergBuildOrder();
}

void BuildOrderData::initStarterBuildOrder()
{
	starter_build_order = clean_build_order;

	starter_build_order[8] = BWAPI::UnitTypes::Protoss_Pylon;
	starter_build_order[10] = BWAPI::UnitTypes::Protoss_Gateway;
	starter_build_order[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	starter_build_order[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
}

void BuildOrderData::initProtossVTerranBuildOrder()
{
	protoss_v_terran = clean_build_order;

	protoss_v_terran[10] = BWAPI::UnitTypes::Protoss_Gateway;
	protoss_v_terran[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	protoss_v_terran[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
	protoss_v_terran[18] = BWAPI::UnitTypes::Protoss_Dragoon;
	protoss_v_terran[26] = BWAPI::UnitTypes::Protoss_Robotics_Facility;
	protoss_v_terran[34] = BWAPI::UnitTypes::Protoss_Observatory;
}

/// <summary>
/// Aggressive https://liquipedia.net/starcraft/Protoss_vs._Zerg_Guide#Branch_II:_Two_Gateways
/// </summary>
void BuildOrderData::initProtossVZergBuildOrder()
{
	protoss_v_zerg = clean_build_order;

	protoss_v_terran[10] = BWAPI::UnitTypes::Protoss_Gateway;
	protoss_v_terran[12] = BWAPI::UnitTypes::Protoss_Gateway;
}

void BuildOrderData::initProtossVProtossBuildOrder()
{
	protoss_v_protoss = clean_build_order;
}
