#include "CombatManager.h"

//#include "MiraBotMain.h"
#include "Global.h"
#include "Tools.h"
#include "CombatData.h"
#include "limits.h"

using namespace MiraBot;

CombatManager::CombatManager() = default;

void CombatManager::onFrame()
{
	// See if we should start rushing

	// Handle idle units

	// Check status of current combat - can we win or should we retreat?
}

void CombatManager::onUnitComplete(BWAPI::Unit unit)
{
	// If unit is attack unit, add to map
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

void CombatManager::onUnitShow(BWAPI::Unit unit)
{
	if (!unit) return;

	if (unit->canAttack() && unit->getPlayer() != BWAPI::Broodwar->self())
	{
		auto area = Global::map().map.GetNearestArea(unit->getTilePosition());
		if (area == Global::map().main_area || area == Global::map().snd_area)
		{
			// If unit is enemy and in our base, attack it!
			for (auto u : m_attack_units_)
			{
				if (fighter_status_map[u] == Enums::defending)
				{
					u->attack(unit);
				}
			}
		}
	}
}

void CombatManager::onUnitHide(BWAPI::Unit unit)
{
}

BWAPI::Position CombatManager::getChokepointToGuard(BWAPI::Unit unit)
{
	// Get chokepoint with fewest assigned units
	auto chokepoints = Global::map().getChokepoints(Global::map().main_area);
	auto closest_cp = BWAPI::Positions::None;
	auto count_closest = INT_MAX;
	for (auto cp : chokepoints)
	{
		// Count how many guards are assigned to this chokepoint
		auto count = 0;
		for (auto& [_, _cp] : guard_map) count += _cp == cp;

		// Choose this chokepoint if it has too few units assigned to it
		if (closest_cp == BWAPI::Positions::None || count < count_closest)
		{
			closest_cp = cp;
			count_closest = count;
		}
	}

	return closest_cp;
}

void CombatManager::addCombatUnit(BWAPI::Unit unit)
{
	m_attack_units_.insert(unit);
	goDefend(unit);
}

void CombatManager::resetCombatUnit(BWAPI::Unit unit)
{
}

void CombatManager::handleIdleDefender(BWAPI::Unit unit)
{
}

void CombatManager::handleIdleRetreater(BWAPI::Unit unit)
{
}

void CombatManager::handleIdleAttacker(BWAPI::Unit unit)
{
}

void CombatManager::setTarget(BWAPI::Unit unit, BWAPI::Unit target)
{
}

void CombatManager::goRetreat(BWAPI::Unit unit)
{
}

void CombatManager::goAttack(BWAPI::Unit unit)
{
}

void CombatManager::goDefend(BWAPI::Unit unit)
{
	// Set defender status
	fighter_status_map[unit] = Enums::defending;

	// Assign chokepoint to unit if not already assigned one
	if (guard_map[unit].x + guard_map[unit].y == 0) guard_map[unit] = getChokepointToGuard(unit);
	auto r = guard_map[unit];

	// Randomize position a bit
	auto x = (rand() % 50) + 75;
	auto y = (rand() % 50) + 75;
	if (rand() & 1) x *= -1;
	if (rand() & 1) y *= -1;

	// Send unit to randomized position near the center of the CP
	const auto attack_pos = guard_map[unit] + BWAPI::Position(x, y);
	unit->attack(attack_pos);
}

BWAPI::Unit CombatManager::getClosestTarget(BWAPI::Unit unit)
{
	return unit;
}
