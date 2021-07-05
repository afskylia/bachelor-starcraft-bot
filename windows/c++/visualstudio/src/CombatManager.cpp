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

	// See if we are under attack/no longer under attack
	updateAttackStatus();

	// Check status of current combat - can we win or should we retreat?
	updateCombatStatus();


	// Handle idle units
	for (auto& u : m_attack_units)
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

	auto it = m_attack_units.find(unit);
	if (it != m_attack_units.end()) m_attack_units.erase(it);

	it = m_defensive_units_.find(unit);
	if (it != m_defensive_units_.end()) m_defensive_units_.erase(it);

	it = m_offensive_units_.find(unit);
	if (it != m_offensive_units_.end()) m_offensive_units_.erase(it);
}

void CombatManager::onUnitShow(BWAPI::Unit unit)
{
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
	auto chokepoints = Global::map().getChokepoints(Global::map().expos.back());
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
	m_attack_units.insert(unit);
	goDefend(unit);
}

void CombatManager::resetTarget(BWAPI::Unit target)
{
	// Remove target from maps
	target_attackers.erase(target);
	targets.erase(target);

	// Remove as target for units currently targeting it
	for (auto& u : m_attack_units)
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
	auto was_attacking = unit->getLastCommand().getType() == BWAPI::UnitCommandTypes::Attack_Unit;
	if (was_attacking && !under_attack)
	{
		std::cout << "Going back to defenses\n";
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
	// check that we are actually there
	if (unit->getDistance(rally_point->Bases()[0].Center()) > 300)
	{
		unit->move(rally_point->Bases()[0].Center());
		return;
	}

	auto idle_retreater_count = 0;
	for (auto* u : m_attack_units)
		if (fighter_status_map[u] == Enums::retreating && u->isIdle())
		{
			idle_retreater_count++;
		}

	if (idle_retreater_count >= (total_rusher_count - lost_rusher_count) * 0.8)
	{
		retreating = false;
	}

	goDefend(unit);
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
	}
	else
	{
		setTarget(unit, target);
	}
}

void CombatManager::handleIdleRallyer(BWAPI::Unit unit)
{
	if (Global::map().map.GetNearestArea(unit->getTilePosition()) == rally_point)
	{
		std::cout << unit->getType() << " idling (at rally point hopefully)\n";
		rallied_rushers++;
		fighter_status_map[unit] = Enums::attacking;
	}
	else
	{
		goRally(unit);
		return;
	}

	if (rallied_rushers == (total_rusher_count - lost_rusher_count))
	{
		ralliedUpNowAttack();
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

	// Stop units that are stuck or their target position is unwalkable
	if (rallying && rallied_rushers > (total_rusher_count - lost_rusher_count) / 2)
	{
		for (auto* u : m_attack_units)
		{
			if (fighter_status_map[u] != Enums::rallying || fighter_status_map[u] != Enums::attacking) continue;
			if (!Global::map().isWalkable(u->getTilePosition()) || u->isStuck())
			{
				std::cout << "u->stop()\n";
				u->stop();
			}
		}
	}

	if (rallying)
	{
		const auto rally_pos = rally_point->Bases()[0].Center();
		for (auto* u : m_attack_units)
			if (u->getPosition().getDistance(rally_pos) > 200 && rally_point != Global::map().map.GetNearestArea(
				BWAPI::TilePosition(u->getPosition())))
				return;

		// Start rushing is everyone is within 200 pixels of rallying point or in area
		//startRushing();
		ralliedUpNowAttack();
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
	//if (rallying == true) lost_rusher_count++;
	lost_rusher_count++;
	fighter_status_map[unit] = Enums::retreating;
	//unit->move(BWAPI::Position(Global::map().expos.front()->Bases()[0].Center()));
	unit->move(rally_point->Bases()[0].Center());
}

void CombatManager::goAttack(BWAPI::Unit unit)
{
	fighter_status_map[unit] = Enums::attacking;

	if (targets.empty())
	{
		unit->attack(rush_target->Bases()[0].Center());
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
	auto units_nearby = BWAPI::Unitset::none;
	for (const auto* base : Global::map().expos)
	{
		auto pos = base->Bases()[0].Center();
		auto u = BWAPI::Broodwar->getUnitsInRadius(pos, 800);
		for (auto uu : u)
		{
			if (!uu->getType().isWorker() && uu->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
			{
				units_nearby.insert(uu);
			}
		}

		// Also add units in neighboring areas
		auto neighbors = base->AccessibleNeighbours();
		for (const auto* neighbor : neighbors)
		{
			if (neighbor->Bases().empty()) continue;
			pos = neighbor->Bases()[0].Center();
			u = BWAPI::Broodwar->getUnitsInRadius(pos, 600);
			for (auto uu : u)
			{
				if (!uu->getType().isWorker() && uu->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
				{
					units_nearby.insert(uu);
				}
			}
		}
	}

	targets.insert(units_nearby.begin(), units_nearby.end());

	// See if we're no longer under attack
	if (under_attack)
	{
		auto has_enemy = false;
		for (auto* u : units_nearby)
		{
			if (u->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
			{
				has_enemy = true;
				break;
			}
		}

		if (!has_enemy)
		{
			std::cout << "No longer under attack\n";
			under_attack = false;
		}
		return;
	}

	// If not currently marked as under attack, check if we ARE under attack
	for (auto* u : units_nearby)
	{
		if (u->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
		{
			std::cout << "We're under attack!\n";
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

	setRushTarget();
	rallied_rushers = 0;
	total_rusher_count = 0;
	lost_rusher_count = 0;

	std::vector<BWAPI::Unit> rush_squad = {};
	auto count = 0;

	// Turn half of our attack units into combat units
	for (auto* u : m_attack_units)
	{
		if (count >= m_attack_units.size() * 0.75) break;
		total_rusher_count++;
		goRally(u);
		count++;
	}
}


void CombatManager::retreatFromCombat()
{
	std::cout << "Start retreating\n";
	attacking = false;
	rallying = false;
	retreating = true;
	lost_rusher_count = 0;
	total_rusher_count = 0;

	for (auto* u : m_attack_units)
	{
		if (fighter_status_map[u] == Enums::attacking || fighter_status_map[u] == Enums::rallying)
		{
			u->stop();
			goRetreat(u);
			std::cout << u->getType() << " retreats from combat\n";
		}
	}
}

void CombatManager::ralliedUpNowAttack()
{
	std::cout << "Rushers grouped up - start attacking!\n";
	rallying = false;
	for (auto* u : m_attack_units)
	{
		if (fighter_status_map[u] == Enums::attacking || fighter_status_map[u] == Enums::rallying)
		{
			goAttack(u);
		}
	}
}

// Get closest target from (visible/known?) targets
BWAPI::Unit CombatManager::chooseTarget(BWAPI::Unit unit, bool same_area)
{
	// same_area: Whether target needs to be in same area as unit or not
	const auto* area = Global::map().map.GetNearestArea(unit->getTilePosition());
	auto neighbors = area->AccessibleNeighbours();

	// First try to find a target that has at most 2 other units assigned to it
	BWAPI::Unitset available_targets = {};
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
	std::cout << "Setting rush targets\n";
	if (Global::information().enemy_areas.empty())
	{
		std::cout << "Enemy areas empty\n";
		return;
	}

	// Pick closest enemy base to our main base
	//rush_target = Global::map().getClosestArea(Global::map().expos.front(), Global::information().enemy_areas);
	rush_target = Global::map().map.GetNearestArea(Global::information().enemy_start_location);

	// Choose rally point
	//const auto target_neighbors = rush_target->AccessibleNeighbours();
	//rally_point = Global::map().getClosestArea(Global::map().expos.front(), target_neighbors);
	//rally_point = Global::map().expos.front(); // Rally at our base

	//rally_point = Global::map().getClosestArea(rush_target, Global::map().expos);
	const auto closest_expo = Global::map().getClosestArea(rush_target, Global::map().expos);
	rally_point = Global::map().getClosestArea(rush_target, closest_expo->AccessibleNeighbours());
}
