#include "BuildOrderData.h"

BuildOrderData::BuildOrderData()
{
}

//8 - Pylon
//10 - Gateway
//12 - Assimilator
//13 - Cybernetics Core
build_order BuildOrderData::SimpleBuildOrder()
{
	build_order order;
	order.steps = {
		{8, BWAPI::UnitTypes::Protoss_Pylon},
		{10, BWAPI::UnitTypes::Protoss_Gateway},
		{12, BWAPI::UnitTypes::Protoss_Assimilator},
		{13, BWAPI::UnitTypes::Protoss_Cybernetics_Core}
	};
	return order;
}