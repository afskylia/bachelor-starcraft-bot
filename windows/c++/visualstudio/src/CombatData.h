#pragma once
#include "BWAPI.h"

class CombatData
{
public:
	enum attacker_job { attack, move, idle };

	BWAPI::UnitType protoss_attack_units[8] = {
		BWAPI::UnitTypes::Protoss_Arbiter,
		BWAPI::UnitTypes::Protoss_Archon,
		BWAPI::UnitTypes::Protoss_Dark_Templar,
		BWAPI::UnitTypes::Protoss_Dragoon,
		BWAPI::UnitTypes::Protoss_Photon_Cannon,
		BWAPI::UnitTypes::Protoss_Reaver,
		BWAPI::UnitTypes::Protoss_Scout,
		BWAPI::UnitTypes::Protoss_Zealot
	};
};
