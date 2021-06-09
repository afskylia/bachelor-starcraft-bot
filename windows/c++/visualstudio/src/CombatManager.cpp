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

	// Randomize position a bit
	auto pp = BWAPI::Broodwar->getBuildLocation(BWAPI::Broodwar->self()->getRace().getSupplyProvider(),
	                                            BWAPI::TilePosition(closest_cp));
	srand(time(nullptr));
	auto x = (rand() % 50) + 75;
	auto y = (rand() % 50) + 75;
	if (rand() & 1) x *= -1;
	if (rand() & 1) y *= -1;
	auto attack_pos = BWAPI::Position(pp) + BWAPI::Position(x, y);
	//auto attack_pos = BWAPI::Position(pp);
	//auto attack_pos = closest_cp + BWAPI::Position(x, y);

	//// this is a hotfix, sometimes (e.g. on Destination.scx) units are sent to an unreachable location...
	//auto fail_count = 0;
	//while (!Global::map().isWalkable(BWAPI::TilePosition(attack_pos)))
	//{
	//	if (fail_count>10)
	//	{
	//		std::cout << "Failed to find walkable position for chokepoint " << closest_cp;
	//		return;
	//	}
	//	x = (rand() % 50) + 75;
	//	y = (rand() % 50) + 75;
	//	if (rand() & 1) x *= -1;
	//	if (rand() & 1) y *= -1;
	//	attack_pos = closest_cp + BWAPI::Position(x, y);
	//	fail_count++;
	//}

	// Assign unit to chokepoint guard and attack-move there
	guard_map[unit] = closest_cp;
	unit->attack(attack_pos);
}

void CombatManager::addCombatUnit(BWAPI::Unit unit)
{
	m_attack_units_.insert(unit);
	guardBase(unit);
}
