#pragma once

#include "BWAPI.h"
#include <set>

namespace MiraBot
{
	class BuildOrderData
	{
	public:
		BuildOrderData();

		// Sorts in reverse order
		struct Cmp
		{
			bool operator ()(const std::pair<double, BWAPI::UnitType>& a,
			                 const std::pair<double, BWAPI::UnitType>& b) const;
		};

		/// <summary>
		/// list of attack units, make sure the numbers add up to 1
		/// </summary>
		std::set<std::pair<double, BWAPI::UnitType>, Cmp> attack_unit_list = {
			{0.2, BWAPI::UnitTypes::Protoss_Zealot},
			{0.8, BWAPI::UnitTypes::Protoss_Dragoon},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> starter_build_order = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{13, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			// Tweaks by us
			{40, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> protoss_v_terran = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{13, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			//{18, std::make_pair(BWAPI::UnitTypes::Protoss_Dragoon, 20)},
			{26, std::make_pair(BWAPI::UnitTypes::Protoss_Robotics_Facility, 1)},
			{34, std::make_pair(BWAPI::UnitTypes::Protoss_Observatory, 1)},
			// Tweaks by us
			{40, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> protoss_v_zerg = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{13, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			// Tweaks by us
			{40, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};

		std::map<int, std::pair<BWAPI::UnitType, int>> protoss_v_protoss = {
			{4, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{10, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 4)},
			{12, std::make_pair(BWAPI::UnitTypes::Protoss_Gateway, 1)},
			//{13, std::make_pair(BWAPI::UnitTypes::Protoss_Nexus, 1)},
			{14, std::make_pair(BWAPI::UnitTypes::Protoss_Assimilator, 1)},
			{15, std::make_pair(BWAPI::UnitTypes::Protoss_Cybernetics_Core, 1)},
			// Tweaks by us
			{40, std::make_pair(BWAPI::UnitTypes::Protoss_Forge, 1)},
			{41, std::make_pair(BWAPI::UnitTypes::Protoss_Fleet_Beacon, 1)},
		};
	};
}
