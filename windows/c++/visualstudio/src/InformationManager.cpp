#include "InformationManager.h"

#include "MiraBotMain.h"
#include "Global.h"

using namespace MiraBot;

/// <summary>
/// Here we save important information through the game e.g. enemy race and where they are.
/// </summary>
InformationManager::InformationManager()
{
	//// Save positions of all chokepoints in main base
	//auto base = BWAPI::Broodwar->self()->getStartLocation();
	//const auto area = Global::map().map.GetNearestArea(base);

	//for (auto cp : area->ChokePoints())
	//	base_chokepoints.push_back(BWAPI::Position(cp->Center()));
}


void InformationManager::onFrame()
{
	if (m_should_update_ && BWAPI::Broodwar->getFrameCount() % 50 == 0)
	{
		informationIsUpdated();
		m_should_update_ = false;
	}
}

/// <summary>
/// Call this when information is updated, e.g. enemy is found
/// </summary>
void InformationManager::informationUpdateShouldHappen()
{
	if (!m_should_update_) m_should_update_ = true;
}

/// <summary>
/// information in the manager is send to e.g. strategy manager to update strategy.
/// </summary>
void InformationManager::informationIsUpdated()
{
	updateEnemyStrategy();
	Global::strategy().informationUpdate();
}

/// <summary>
/// TODO support for expanding
/// </summary>
void InformationManager::updateEnemyStrategy()
{
	// Do not update if we do not know any enemies.
	if (enemy_units.empty()) return;

	for (BWAPI::UnitInterface* enemy_unit : enemy_units)
	{
		if (enemy_unit->getPosition().getApproxDistance(main_base->getPosition()) < 1000)
		{
			m_current_enemy_strategy = Enums::offensive;
			return;
		}
	}

	m_current_enemy_strategy = Enums::defensive;
}

/// <summary>
/// log the enemy race and starting location
/// </summary>
/// <param name="unit">First enemy unit found</param>
void InformationManager::logEnemyRaceAndStartLocation(BWAPI::Unit unit)
{
	// If we did not find the enemy, log it
	if (!found_enemy)
	{
		enemy_race = unit->getType().getRace();
		found_enemy = true;
		std::cout << "Enemy is " << enemy_race << "\n";

		auto& start_locations = BWAPI::Broodwar->getStartLocations();

		// Find closest starting location to enemy unit
		// TODO: Also save locations of other enemy bases they might build later in the game
		double shortest_distance = INT_MAX;
		for (BWAPI::TilePosition position : start_locations)
		{
			const auto distance = position.getDistance(unit->getTilePosition());
			if (distance < shortest_distance)
			{
				shortest_distance = distance;
				enemy_start_location = position;
			}
		}
		std::cout << "Enemy starting location: " << enemy_start_location << "\n";

		informationUpdateShouldHappen();
	}
}

/// <summary>
/// Keeps track of the enemy units.
/// </summary>
/// <param name="unit">the unit to add or remove</param>
/// <param name="remove_unit">should we remove the unit, default is false</param>
void InformationManager::addOrRemoveEnemyUnit(BWAPI::Unit unit, bool remove_unit)
{
	// try find the unit in unit set
	auto it = enemy_units.find(unit);

	// if not in the unit set, add it, if in the set and should remove then remove it 
	if (it == enemy_units.end())
	{
		enemy_units.insert(unit);
		informationUpdateShouldHappen();
	}
	else if (remove_unit)
	{
		enemy_units.erase(it);
		informationUpdateShouldHappen();
	}
}

void InformationManager::onUnitShow(BWAPI::Unit unit)
{
	// If we find an enemy, we should log the information
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
	{
		logEnemyRaceAndStartLocation(unit);
		addOrRemoveEnemyUnit(unit);
		informationUpdateShouldHappen();
	}
}

void InformationManager::onStart()
{
	main_base = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()),
	                                            BWAPI::Filter::IsResourceDepot);
	informationUpdateShouldHappen();
}

void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	// If we find an enemy, we should log the information
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
	{
		addOrRemoveEnemyUnit(unit, true);
		informationUpdateShouldHappen();
	}
}


/// <summary>
/// Gets the strategy from enemy units
/// </summary>
/// <returns>strategy based on units</returns>
Enums::strategy_type InformationManager::getEnemyStrategy()
{
	return m_current_enemy_strategy;
}
