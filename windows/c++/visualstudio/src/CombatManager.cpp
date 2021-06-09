#include "CombatManager.h"

//#include "MiraBotMain.h"
#include "Global.h"
#include "Tools.h"
#include "CombatData.h"
#include "limits.h";

using namespace MiraBot;

CombatManager::CombatManager()
{
}

void CombatManager::onFrame()
{
}

void CombatManager::onUnitComplete(BWAPI::Unit unit)
{
	// An attack unit
	for (auto protoss_attack_unit : m_combat_data_.protoss_attack_units)
	{
		if (unit->getType() == protoss_attack_unit)
		{
			addCombatUnit(unit);
			return;
		}
	}
}

void CombatManager::onUnitDestroy(BWAPI::Unit unit)
{
	// Erase unit from maps and unit sets
	guard_map.erase(unit);

	auto it = m_attack_units_.find(unit);
	if (it != m_attack_units_.end()) m_attack_units_.erase(it);

	it = m_defensive_units_.find(unit);
	if (it != m_defensive_units_.end()) m_defensive_units_.erase(it);

	it = m_offensive_units_.find(unit);
	if (it != m_offensive_units_.end()) m_offensive_units_.erase(it);
}

// Send the unit to a chokepoint in the main base and stand ready
void CombatManager::guardBase(BWAPI::Unit unit)
{
	// Get chokepoint with fewest assigned units
	auto chokepoints = Global::information().base_chokepoints;
	auto closest_cp = BWAPI::Positions::None;
	auto count_closest = INT_MAX;
	for (auto cp : chokepoints)
	{
		// Count how many guards are assigned to this chokepoint
		auto count = 0;
		for (auto& p : guard_map) count += p.second == cp;
		if (closest_cp == BWAPI::Positions::None || count < count_closest)
			closest_cp = cp;
	}

	// Assign unit to chokepoint guard and attack-move there
	guard_map[unit] = closest_cp;

	unit->attack(closest_cp);
}

void CombatManager::addCombatUnit(BWAPI::Unit unit)
{
	m_attack_units_.insert(unit);
	guardBase(unit);
}
