#pragma once
#include <iostream>
#include <BWAPI/Unit.h>
#include "CombatData.h"

#include "Tools.h"


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
	};
}
