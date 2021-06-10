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
	//auto chokepoints = Global::information().base_chokepoints;
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

	// Randomize position a bit
	const auto supply_provider = BWAPI::Broodwar->self()->getRace().getSupplyProvider();
	const auto tile_pos = BWAPI::Broodwar->getBuildLocation(supply_provider, BWAPI::TilePosition(closest_cp));

	srand(time(nullptr));
	auto x = (rand() % 50) + 75;
	auto y = (rand() % 50) + 75;
	if (rand() & 1) x *= -1;
	if (rand() & 1) y *= -1;

	// TODO: Choose one of the following placement options for guards
	// A) Send unit to non-randomized pylon position
	const auto option_a = BWAPI::Position(tile_pos);

	// B) Send unit to randomized pylon position
	const auto option_b = BWAPI::Position(tile_pos) + BWAPI::Position(x, y);

	// C) Send unit to randomized position near the center of the CP - not using pylon placement
	const auto option_c = closest_cp + BWAPI::Position(x, y);

	const auto attack_pos = option_c;

	// TODO: Fix bug where units try to walk to unreachable chokepoint (e.g. on (2)Destination.scx)
	//auto fail_count = 0;
	//while (!Global::map().isWalkable(BWAPI::TilePosition(attack_pos)))
	//{
	//	if (fail_count>10)
	//	{
	//		std::cout << "Failed to find walkable position for chokepoint " << closest_cp;
	//		return;
	//	}
	//	...
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
