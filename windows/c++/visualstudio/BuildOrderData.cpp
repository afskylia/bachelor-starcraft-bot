#include "BuildOrderData.h"

BuildOrderData::BuildOrderData()
{
}

//8 - Pylon
//10 - Gateway
//12 - Assimilator
//13 - Cybernetics Core
BuildOrder BuildOrderData::SimpleBuildOrder()
{
	BuildOrder order;
	order.steps[8] = BWAPI::UnitTypes::Protoss_Pylon;
	order.steps[10] = BWAPI::UnitTypes::Protoss_Gateway;
	order.steps[12] = BWAPI::UnitTypes::Protoss_Assimilator;
	order.steps[13] = BWAPI::UnitTypes::Protoss_Cybernetics_Core;
}