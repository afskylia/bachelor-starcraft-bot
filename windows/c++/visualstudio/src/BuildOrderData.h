#pragma once

#include "BWAPI.h"

namespace MiraBot
{
	class BuildOrderData
	{
	public:
		BuildOrderData();

		std::map<int, std::pair<BWAPI::UnitType, int>> starter_build_order = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{11, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{13, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			{15, std::make_pair(BWAPI::UnitTypes::Protoss_Photon_Cannon, 1)},
			// Tweaks by us
			//{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> protoss_v_terran = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{11, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{13, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			{15, std::make_pair(BWAPI::UnitTypes::Protoss_Photon_Cannon, 3)},
			{26, std::make_pair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1)},
			{34, std::make_pair(BWAPI::UnitTypes::Protoss_Observatory, 1)},
			// Tweaks by us
			//{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> protoss_v_zerg = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{11, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{13, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			{15, std::make_pair(BWAPI::UnitTypes::Protoss_Photon_Cannon, 1)},
			// Tweaks by us
			//{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> protoss_v_protoss = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{11, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{14, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{15, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			{16, std::make_pair(BWAPI::UnitTypes::Protoss_Photon_Cannon, 1)},
			// Tweaks by us
			//{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};
	};
}
