#pragma once

#include "BWAPI.h"


namespace MiraBot
{
	class BuildOrderData
	{
	public:
		BuildOrderData();

		std::map<int, BWAPI::UnitType> starter_build_order = {
			{4, BWAPI::UnitTypes::Protoss_Nexus},
			{10, BWAPI::UnitTypes::Protoss_Gateway},
			{12, BWAPI::UnitTypes::Protoss_Assimilator},
			{13, BWAPI::UnitTypes::Protoss_Cybernetics_Core},
		};

		std::map<int, BWAPI::UnitType> protoss_v_terran = {
			{4, BWAPI::UnitTypes::Protoss_Nexus},
			{10, BWAPI::UnitTypes::Protoss_Gateway},
			{12, BWAPI::UnitTypes::Protoss_Assimilator},
			{13, BWAPI::UnitTypes::Protoss_Cybernetics_Core},
			{18, BWAPI::UnitTypes::Protoss_Dragoon},
			{26, BWAPI::UnitTypes::Protoss_Robotics_Facility},
			{34, BWAPI::UnitTypes::Protoss_Observatory},
		};

		std::map<int, BWAPI::UnitType> protoss_v_zerg = {
			{4, BWAPI::UnitTypes::Protoss_Nexus},
			{10, BWAPI::UnitTypes::Protoss_Gateway},
			{12, BWAPI::UnitTypes::Protoss_Gateway},
		};

		std::map<int, BWAPI::UnitType> protoss_v_protoss = {
			{4, BWAPI::UnitTypes::Protoss_Nexus},
			{10, BWAPI::UnitTypes::Protoss_Gateway},
			{12, BWAPI::UnitTypes::Protoss_Gateway},
			{13, BWAPI::UnitTypes::Protoss_Nexus},
			{14, BWAPI::UnitTypes::Protoss_Assimilator},
			{15, BWAPI::UnitTypes::Protoss_Cybernetics_Core},
		};
	};
}
