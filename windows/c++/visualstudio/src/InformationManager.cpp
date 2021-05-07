#include "InformationManager.h"

#include "MiraBotMain.h"
#include "Global.h"

using namespace MiraBot;

/// <summary>
/// Here we save important information through the game e.g. enemy race and where they are.
/// </summary>
InformationManager::InformationManager() = default;

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
	}
	else if (remove_unit)
	{
		enemy_units.erase(it);
	}
}

void InformationManager::onUnitShow(BWAPI::Unit unit)
{
	// If we find an enemy, we should log the information
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
	{
		logEnemyRaceAndStartLocation(unit);
		addOrRemoveEnemyUnit(unit);
	}
}

void InformationManager::onStart()
{
	main_base = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()),
	                                            BWAPI::Filter::IsResourceDepot);
}

void InformationManager::onUnitDestroy(BWAPI::Unit unit)
{
	// If we find an enemy, we should log the information
	if (unit->getPlayer()->isEnemy(BWAPI::Broodwar->self()))
	{
		addOrRemoveEnemyUnit(unit, true);
	}
}
