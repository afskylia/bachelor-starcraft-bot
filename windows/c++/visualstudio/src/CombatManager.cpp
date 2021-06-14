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
	// Clean up targets that have gone invisible (e.g. in fog of war)
	cleanUpTargets();

	updateAttackStatus();

	// See if we should start rushing
	if (!attacking && Global::strategy().shouldStartRushing())
	{
		std::cout << "Start rushing!\n";
		//startRushing();
	}

	// Handle idle units
	for (auto& u : m_attack_units_)
	{
		if (u->isIdle())
		{
			switch (fighter_status_map[u])
			{
			case Enums::defending:
				handleIdleDefender(u);
				break;
			case Enums::attacking:
				handleIdleAttacker(u);
				break;
			case Enums::retreating:
				handleIdleRetreater(u);
				break;
			}
		}
	}

	// Check status of current combat - can we win or should we retreat?
	updateCombatStatus();
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
	// if enemy unit: remove as target
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		resetTarget(unit);

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
	auto a = unit->getType();
	// If enemy unit, add to targets so it will be attacked by the defense squad
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
	{
		targets.insert(unit);
		target_attackers[unit] = 0;

		//auto area = Global::map().map.GetNearestArea(unit->getTilePosition());
		//if (area == Global::map().main_area || area == Global::map().snd_area)
		//{
		//	// We are under attack! Unless it's just a scout.
		//	if (!unit->getType().isWorker()) under_attack = true;
		//}
	}
}

void CombatManager::onUnitHide(BWAPI::Unit unit)
{
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		resetTarget(unit);
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

void CombatManager::resetTarget(BWAPI::Unit target)
{
	// Remove target from maps
	target_attackers.erase(target);
	targets.erase(target);

	// Remove as target for units currently targeting it
	for (auto& u : m_attack_units_)
	{
		if (fighter_target_map[u] == target)
		{
			//fighter_target_map[u] = nullptr;
			fighter_target_map.erase(u);
			u->stop();
		}
	}
}

void CombatManager::resetCombatUnit(BWAPI::Unit unit)
{
	// TODO: remove from maps and stuff
}

void CombatManager::removeUnitTarget(BWAPI::Unit unit)
{
	// Reset unit target
	const auto prev_target = fighter_target_map[unit];
	if (prev_target) target_attackers[prev_target]--;
	//fighter_target_map[unit] = nullptr;
	fighter_target_map.erase(unit);
}

void CombatManager::handleIdleDefender(BWAPI::Unit unit)
{
	// If last command was an attack, and we're no longer under attack, send to guard position again
	if (!under_attack && fighter_target_map[unit])
	{
		removeUnitTarget(unit);
		goDefend(unit);
	}
	else if (under_attack && !targets.empty())
	{
		// Set new target
		auto* const target = chooseTarget(unit, true);
		setTarget(unit, target);
	}
}

void CombatManager::handleIdleRetreater(BWAPI::Unit unit)
{
	goDefend(unit);
}

void CombatManager::handleIdleAttacker(BWAPI::Unit unit)
{
	// Remove current target
	removeUnitTarget(unit);

	// Set new target
	auto* const target = chooseTarget(unit);
	if (!target) goAttack(unit, rush_target_pos);

	setTarget(unit, target);
}

void CombatManager::updateCombatStatus()
{
	// Check how the battle is going - should we retreat?
	// TODO
}

void CombatManager::setTarget(BWAPI::Unit unit, BWAPI::Unit target)
{
	// Set target and attack it
	target_attackers[target]++;
	fighter_target_map[unit] = target;
	unit->attack(target);
}

void CombatManager::goRetreat(BWAPI::Unit unit)
{
	fighter_status_map[unit] = Enums::retreating;
	unit->attack(BWAPI::Position(Global::map().snd_area->BottomRight()));
}

void CombatManager::goAttack(BWAPI::Unit unit)
{
	if (targets.empty())
	{
		goAttack(unit, rush_target_pos);
		return;
	}
	fighter_status_map[unit] = Enums::attacking;
	auto* const target = chooseTarget(unit);
	setTarget(unit, target);
}

void CombatManager::goAttack(BWAPI::Unit unit, BWAPI::Position target_pos)
{
	fighter_status_map[unit] = Enums::attacking;
	unit->attack(target_pos);
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

void CombatManager::updateAttackStatus()
{
	auto fst = Global::map().main_area->Minerals()[0]->Unit();
	auto snd = Global::map().snd_area->Minerals()[0]->Unit();

	auto u1 = fst->getUnitsInRadius(600);
	auto u2 = snd->getUnitsInRadius(600);
	u1.insert(u2.begin(), u2.end());

	if (under_attack)
	{
		auto has_enemy = false;
		for (auto u : u1)
		{
			if (u->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
			{
				has_enemy = true;
				break;
			}
		}

		if (!has_enemy) under_attack = false;
		return;
	}

	// If not currently marked as under attack, check if we ARE under attack
	for (auto u : u1)
	{
		if (u->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		{
			under_attack = true;
			return;
		}
	}
}

void CombatManager::cleanUpTargets()
{
	std::vector<BWAPI::Unit> to_reset = {};
	for (auto& t : targets)
		if (!t->isVisible()) to_reset.push_back(t);

	for (auto t : to_reset)
		resetTarget(t);
}

void CombatManager::startRushing()
{
	auto enemy_base = Global::information().enemy_start_location;
	rush_target_pos = BWAPI::Position(enemy_base);

	std::vector<BWAPI::Unit> rush_squad = {};
	auto count = 0;
	// Turn half of our attack units into combat units
	for (auto* u : m_attack_units_)
	{
		if (count >= m_attack_units_.size() / 2) break;
		goAttack(u, rush_target_pos);
		std::cout << u->getType() << " is rushing\n";
		count++;
	}

	// TODO: re-position guards so chokepoints remain equally guarded
}

void CombatManager::retreatFromCombat()
{
	for (auto* u : m_attack_units_)
	{
		if (fighter_status_map[u] == Enums::attacking)
		{
			goRetreat(u);
			std::cout << u->getType() << " retreats from combat\n";
		}
	}
}

// Get closest target from (visible/known?) targets
BWAPI::Unit CombatManager::chooseTarget(BWAPI::Unit unit, bool same_area)
{
	// same_area: Whether target needs to be in same area as unit
	const auto* area = Global::map().map.GetNearestArea(unit->getTilePosition());

	// First try to find a target that has at most 2 other units assigned to it
	BWAPI::Unitset available_targets = {};

	for (auto& t : targets)
	{
		if (target_attackers[t] > 4) continue;

		const auto target_area = Global::map().map.GetNearestArea(t->getTilePosition());
		if (same_area && area != target_area) continue;
		available_targets.insert(t);
	}

	// Return the closest target (out of all targets if no targets w. <=2 units assigned)
	if (available_targets.empty() && !same_area) return Tools::getClosestUnitTo(unit->getPosition(), targets);
	if (available_targets.empty()) return nullptr;
	return Tools::getClosestUnitTo(unit->getPosition(), available_targets);
}
