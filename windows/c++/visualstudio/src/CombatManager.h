#pragma once
#include <iostream>
#include <BWAPI/Unit.h>
#include "CombatData.h"

#include "Tools.h"
#include "Enums.h"

using namespace Enums;

namespace MiraBot
{
	class CombatManager
	{
		friend class Global;

		BWAPI::Unitset m_attack_units_ = BWAPI::Unitset::none;
		BWAPI::Unitset m_defensive_units_ = BWAPI::Unitset::none;
		BWAPI::Unitset m_offensive_units_ = BWAPI::Unitset::none;
		CombatData m_combat_data_;


	public:

		CombatManager();
		void onFrame();
		void onUnitComplete(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);
		void guardBase(BWAPI::Unit unit);
		void addCombatUnit(BWAPI::Unit unit);

		// Map of chokepoints and a vector of units currently guarding the chokepoint
		//std::map<BWAPI::Position, std::vector<BWAPI::Unit>> guard_map = {};
		std::map<BWAPI::Unit, BWAPI::Position> guard_map = {};

		// Map of attack units and their status
		std::map<BWAPI::Unit, combat_status> fighter_status_map;
	};
}
