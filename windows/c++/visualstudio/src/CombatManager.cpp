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
	// TODO: Don't call onFrame on every single frame

	// Clean up targets that have gone invisible (e.g. in fog of war)
	cleanUpTargets();

	// See if we are under attack/no longer under attack
	updateAttackStatus();

	// Check status of current combat - can we win or should we retreat?
	updateCombatStatus();


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
			case Enums::rallying:
				handleIdleRallyer(u);
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

	// See if we should start rushing
	if (!attacking && !rallying && !retreating && Global::strategy().shouldStartRushing())
		startRushing();
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
	{
		resetTarget(unit);
		return;
	}

	if ((attacking || rallying) && (fighter_status_map[unit] == Enums::attacking || fighter_status_map[unit] ==
		Enums::rallying))
		lost_rusher_count++;


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
			//u->stop();
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
	if (unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Move)
	{
		std::cout << "BING\n";
	}
	// If last command was an attack, and we're no longer under attack, send to guard position again
	auto was_attacking = unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit;
	if (was_attacking && !under_attack)
	{
		std::cout << "yokes\n";
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

	for (auto u : m_attack_units_)
		if (fighter_status_map[u] == retreating) return;
	retreating = false;
}

void CombatManager::handleIdleAttacker(BWAPI::Unit unit)
{
	if (rallying) return;

	// Remove current target
	removeUnitTarget(unit);

	// Set new target
	auto* const target = chooseTarget(unit);
	if (!target)
	{
		goAttack(unit);
		//goAttack(unit, rush_target_pos);
		/*goRetreat(unit);
		lost_rusher_count++;
		return;*/
	}

	setTarget(unit, target);
}

void CombatManager::handleIdleRallyer(BWAPI::Unit unit)
{
	if (Global::map().map.GetNearestArea(unit->getTilePosition()) == rally_point)
	{
		std::cout << unit->getType() << " idling (at rally point hopefully)\n";
		rallied_rushers++;
		fighter_status_map[unit] = Enums::attacking;
	}

	if (rallied_rushers == (total_rusher_count - lost_rusher_count))
	{
		rallying = false;
		std::cout << "Rushers grouped up - start attacking!\n";
		for (auto u : m_attack_units_)
		{
			if (fighter_status_map[u] == Enums::attacking)
			{
				goAttack(u);
			}
		}
	}
}

void CombatManager::updateCombatStatus()
{
	// Check how the battle is going - should we retreat?
	if (!attacking) return;

	// TODO: Check if we're outnumbered by enemy attack units (don't count workers)
	/*auto t = targets
	auto ratio = (total_rusher_count-lost_rusher_count)*/

	if (lost_rusher_count >= total_rusher_count * 0.33)
	{
		std::cout << "lost_rushers >= initial_rushers*0.4\n";
		retreatFromCombat();
		return;
	}

	if (rallying) return;

	auto idle_count = 0;
	for (auto u : m_attack_units_)
	{
		if (fighter_status_map[u] != Enums::attacking) continue;
		if (!u->isIdle()) continue;
		if (targets.empty()) continue;

		idle_count++;
		auto a = u->isMoving();
		auto b = u->isAttacking();
		auto c = u->isIdle();
		auto d = u->getLastCommand().getType();
		std::cout << a << ", " << b << ", " << c << ", " << d << "\n";
	}

	if (idle_count >= (total_rusher_count - lost_rusher_count) / 2)
	{
		std::cout << "idle_count >= rushers\n";
		retreatFromCombat();
	}
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
	unit->move(BWAPI::Position(Global::map().snd_area->BottomRight()));
}

void CombatManager::goAttack(BWAPI::Unit unit)
{
	fighter_status_map[unit] = Enums::attacking;

	if (targets.empty())
	{
		//std::cout << "TARGETS EMPTY\n";
		//goAttack(unit, rush_target_pos);
		unit->attack(rush_target->Bases()[0].Center());
		//goRetreat(unit);
		//return;
		return;
	}
	auto* const target = chooseTarget(unit);
	setTarget(unit, target);
}

void CombatManager::goAttack(BWAPI::Unit unit, BWAPI::Position target_pos)
{
	fighter_status_map[unit] = Enums::attacking;
	unit->attack(target_pos);
}

void CombatManager::goRally(BWAPI::Unit unit)
{
	std::cout << unit->getType() << " going to rally point\n";
	fighter_status_map[unit] = Enums::rallying;
	auto pos = rally_point->Bases()[0].Center();
	unit->attack(pos);
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
	unit->move(attack_pos);
}

void CombatManager::updateAttackStatus()
{
	const auto fst_pos = Global::map().main_area->Bases()[0].Center();
	const auto snd_pos = Global::map().snd_area->Bases()[0].Center();

	auto u1 = BWAPI::Broodwar->getUnitsInRadius(fst_pos, 600);
	auto u2 = BWAPI::Broodwar->getUnitsInRadius(snd_pos, 600);

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
	std::cout << "Start rushing!\n";

	attacking = true;
	rallying = true;
	//rallying = true;
	/*auto enemy_base = Global::information().enemy_start_location;
	rush_target_pos = BWAPI::Position(enemy_base);*/

	setRushTarget();
	rallied_rushers = 0;
	total_rusher_count = 0;
	lost_rusher_count = 0;

	std::vector<BWAPI::Unit> rush_squad = {};
	auto count = 0;
	// Turn half of our attack units into combat units
	for (auto* u : m_attack_units_)
	{
		if (count >= m_attack_units_.size() * 0.75) break;
		total_rusher_count++;
		goRally(u);
		//goAttack(u, rush_target_pos);
		std::cout << u->getType() << " is rushing\n";
		count++;
	}

	std::cout << "Rush start: " << total_rusher_count << ", " << lost_rusher_count << "\n";

	// TODO: re-position guards so chokepoints remain equally guarded
}


void CombatManager::retreatFromCombat()
{
	std::cout << "Start retreating\n";
	attacking = false;
	rallying = false;
	retreating = true;
	lost_rusher_count = 0;
	total_rusher_count = 0;

	for (auto* u : m_attack_units_)
	{
		if (fighter_status_map[u] == Enums::attacking || fighter_status_map[u] == Enums::rallying)
		{
			u->stop();
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
	auto neighbors = area->AccessibleNeighbours();

	// First try to find a target that has at most 2 other units assigned to it
	BWAPI::Unitset available_targets = {};
	// TODO: Global::information().enemy_units i stedet for targets

	for (auto& t : targets)
	{
		if (target_attackers[t] > 4) continue;

		const auto target_area = Global::map().map.GetNearestArea(t->getTilePosition());

		if (same_area && area != target_area && std::find(neighbors.begin(), neighbors.end(), target_area) ==
			neighbors.end())
			continue;

		available_targets.insert(t);
	}

	// Return the closest target (out of all targets if no targets w. <=2 units assigned)
	if (available_targets.empty() && !same_area) return Tools::getClosestUnitTo(unit->getPosition(), targets);
	if (available_targets.empty()) return nullptr;
	return Tools::getClosestUnitTo(unit->getPosition(), available_targets);
}

void CombatManager::setRushTarget()
{
	// TODO

	if (Global::information().enemy_areas.empty())
	{
		std::cout << "Enemy areas empty\n";
		return;
	}

	// TODO pick closest from enemy_areas
	rush_target = Global::map().getClosestArea(Global::map().main_area, Global::information().enemy_areas);

	// Choose rally point
	const auto target_neighbors = rush_target->AccessibleNeighbours();
	rally_point = Global::map().getClosestArea(Global::map().main_area, target_neighbors);

	auto target_pos = rush_target->Bases()[0].Center();
	auto rally_pos = rally_point->Bases()[0].Center();

	Global::map().addCircle(target_pos, "TARGET");
	Global::map().addCircle(target_pos, "RALLY POINT");
}
