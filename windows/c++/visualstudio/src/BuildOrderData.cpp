#include "BuildOrderData.h"


BuildOrderData::BuildOrderData()
{
	for (int i = 0; i <= 200; ++i)
	{
		// TODO: Revamp of build order system so it doesn't have probes on empty levels
		starter_build_order[i] = BWAPI::UnitTypes::Protoss_Probe;
	}
	starter_build_order[4] = BWAPI::UnitTypes::Protoss_Nexus;
	starter_build_order[8] = BWAPI::UnitTypes::Protoss_Pylon;
	starter_build_order[10] = BWAPI::UnitTypes::Protoss_Gateway;
	starter_build_order[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	starter_build_order[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
}
