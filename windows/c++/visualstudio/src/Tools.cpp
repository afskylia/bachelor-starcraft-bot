#include "Tools.h"
#include "MiraBotMain.h"
#include "WorkerManager.h"

// Return vector of units in ascending order from distance to given unit
std::vector<BWAPI::Unit> Tools::sortUnitsByClosest(BWAPI::Unit unit, const BWAPI::Unitset& unitSet)
{
	return sortUnitsByClosest(unit->getPosition(), unitSet);
}

// Return vector of units in ascending order from distance to given position
std::vector<BWAPI::Unit> Tools::sortUnitsByClosest(BWAPI::Position pos, const BWAPI::Unitset& unitSet)
{
	if (unitSet.empty())
		return {};

	std::vector<BWAPI::Unit> units(unitSet.size());
	std::copy(unitSet.begin(), unitSet.end(), units.begin());

	// Bubble sort by closest position to unit
	for (unsigned int i = 0; i < units.size() - 1; i++)
	{
		for (unsigned int j = 0; j < units.size() - i - 1; j++)
		{
			if (units.at(j)->getDistance(pos) > units.at(i)->getDistance(pos))
			{
				// Swap elements
				auto* temp = units.at(j);
				units.at(j) = units.at(i);
				units.at(i) = temp;
			}
		}
	}

	return units;
}

BWAPI::Unit Tools::getClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units)
{
	BWAPI::Unit closest_unit = nullptr;

	for (auto& u : units)
	{
		if (!closest_unit || u->getDistance(p) < u->getDistance(closest_unit))
		{
			closest_unit = u;
		}
	}

	return closest_unit;
}

BWAPI::Unit Tools::getClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units)
{
	if (!unit) { return nullptr; }
	return getClosestUnitTo(unit->getPosition(), units);
}

// Default: Counts all the player's completed units (including non-idle)
int Tools::countUnitsOfType(BWAPI::UnitType type, bool completed, bool idle, BWAPI::Unitset units)
{
	int sum = 0;
	for (auto& unit : units)
	{
		if (completed && !unit->isCompleted()) continue;
		if (idle && !unit->isIdle()) continue;
		if (unit->getType() == type) sum++; //  && unit->getPlayer() == BWAPI::Broodwar->self()
	}
	return sum;
}


BWAPI::Unit Tools::getUnitOfType(BWAPI::UnitType type, bool idle, bool completed)
{
	// For each unit that we own
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() != type) continue;
		if (idle && !unit->isIdle()) continue;
		if (completed && !unit->isCompleted()) continue;
		return unit;
	}

	// If we didn't find a valid unit to return, make sure we return nullptr
	return nullptr;
}

// Default: Returns a vector of completed player-owned units of given type, including non-idle
std::vector<BWAPI::Unit> Tools::getUnitsOfType(BWAPI::UnitType type, bool idle, bool completed)
{
	std::vector<BWAPI::Unit> units = {};
	for (const auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType() != type) continue;
		if (idle && !unit->isIdle()) continue;
		if (completed && !unit->isCompleted()) continue;
		units.push_back(unit);
	}

	return units;
}

BWAPI::Unit Tools::getDepot()
{
	const BWAPI::UnitType depot = BWAPI::Broodwar->self()->getRace().getResourceDepot();
	return getUnitOfType(depot, true);
}

void Tools::drawUnitBoundingBoxes()
{
	for (auto& unit : BWAPI::Broodwar->getAllUnits())
	{
		BWAPI::Position topLeft(unit->getLeft(), unit->getTop());
		BWAPI::Position bottomRight(unit->getRight(), unit->getBottom());
		BWAPI::Broodwar->drawBoxMap(topLeft, bottomRight, BWAPI::Colors::White);
	}
}

void Tools::smartRightClick(BWAPI::Unit unit, BWAPI::Unit target)
{
	// if there's no valid unit, ignore the command
	if (!unit || !target) { return; }

	// Don't issue a 2nd command to the unit on the same frame
	if (unit->getLastCommandFrame() >= BWAPI::Broodwar->getFrameCount()) { return; }

	// If we are issuing the same type of command with the same arguments, we can ignore it
	// Issuing multiple identical commands on successive frames can lead to bugs
	if (unit->getLastCommand().getTarget() == target) { return; }

	// If there's nothing left to stop us, right click!
	unit->rightClick(target);
}

int Tools::getTotalSupply(bool in_progress)
{
	// start the calculation by looking at our current completed supplyt
	int total_supply = BWAPI::Broodwar->self()->supplyTotal();

	// if we don't want to calculate the supply in progress, just return that value
	if (!in_progress) { return total_supply; }

	// if we do care about supply in progress, check all the currently constructing units if they will add supply
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		// ignore units that are fully completed
		if (unit->isCompleted()) { continue; }

		// if they are not completed, then add their supply provided to the total supply
		total_supply += unit->getType().supplyProvided();
	}

	// one last tricky case: if a unit is currently on its way to build a supply provider, add it
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		// get the last command given to the unit
		const BWAPI::UnitCommand& command = unit->getLastCommand();

		// if it's not a build command we can ignore it
		if (command.getType() != BWAPI::UnitCommandTypes::Build) { continue; }

		// add the supply amount of the unit that it's trying to build
		total_supply += command.getUnitType().supplyProvided();
	}

	return total_supply;
}

int Tools::getTotalUsedSupply(bool in_progress_and_building_supply)
{
	// start the calculation by looking at our current completed supply
	int total_supply = BWAPI::Broodwar->self()->supplyUsed();

	// if we don't want to calculate the supply in progress, just return that value
	if (!in_progress_and_building_supply) { return total_supply; }

	// if we do care about supply in progress, check all the currently constructing units if they will add supply
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		// ignore units that are fully completed
		if (unit->isCompleted()) { continue; }

		// if they are not completed, then add their supply provided to the total supply
		total_supply += unit->getType().supplyProvided();
	}

	// one last tricky case: if a unit is currently on its way to build a supply provider, add it
	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		// get the last command given to the unit
		const BWAPI::UnitCommand& command = unit->getLastCommand();

		// if it's not a build command we can ignore it
		if (command.getType() != BWAPI::UnitCommandTypes::Build) { continue; }

		// add the supply amount of the unit that it's trying to build
		total_supply += command.getUnitType().supplyProvided();
	}

	for (auto& unit : BWAPI::Broodwar->self()->getUnits())
	{
		if (unit->getType().isBuilding()) total_supply = 2 + total_supply;
	}

	return total_supply;
}

void Tools::drawUnitHealthBars()
{
	// how far up from the unit to draw the health bar
	int vertical_offset = -10;

	// draw a health bar for each unit on the map
	for (auto& unit : BWAPI::Broodwar->getAllUnits())
	{
		// determine the position and dimensions of the unit
		const BWAPI::Position& pos = unit->getPosition();
		int left = pos.x - unit->getType().dimensionLeft();
		int right = pos.x + unit->getType().dimensionRight();
		int top = pos.y - unit->getType().dimensionUp();
		int bottom = pos.y + unit->getType().dimensionDown();

		// if it's a resource, draw the resources remaining
		if (unit->getType().isResourceContainer() && unit->getInitialResources() > 0)
		{
			double mineral_ratio = static_cast<double>(unit->getResources()) / static_cast<double>(unit->
				getInitialResources());
			drawHealthBar(unit, mineral_ratio, BWAPI::Colors::Cyan, 0);
		}
			// otherwise if it's a unit, draw the hp 
		else if (unit->getType().maxHitPoints() > 0)
		{
			double hp_ratio = static_cast<double>(unit->getHitPoints()) / static_cast<double>(unit->getType().
				maxHitPoints());
			BWAPI::Color hpColor = BWAPI::Colors::Green;
			if (hp_ratio < 0.66) hpColor = BWAPI::Colors::Orange;
			if (hp_ratio < 0.33) hpColor = BWAPI::Colors::Red;
			drawHealthBar(unit, hp_ratio, hpColor, 0);

			// if it has shields, draw those too
			if (unit->getType().maxShields() > 0)
			{
				double shield_ratio = static_cast<double>(unit->getShields()) / static_cast<double>(unit->getType().
					maxShields());
				drawHealthBar(unit, shield_ratio, BWAPI::Colors::Blue, -3);
			}
		}
	}
}

void Tools::drawPoint(BWAPI::Position pos)
{
	BWAPI::Broodwar->drawCircle(BWAPI::CoordinateType::Map,
	                            pos.x, pos.y,
	                            100, BWAPI::Color(0, 255, 0));
}

// TODO: Support multiple bases
void Tools::drawEnemyBases(BWAPI::TilePosition enemyStartLocation)
{
	BWAPI::Broodwar->drawCircle(BWAPI::CoordinateType::Map,
	                            enemyStartLocation.x * 32, enemyStartLocation.y * 32,
	                            200, BWAPI::Color(0, 255, 0));
}


void Tools::drawHealthBar(BWAPI::Unit unit, double ratio, BWAPI::Color color, int yOffset)
{
	int vertical_offset = -10;
	const BWAPI::Position& pos = unit->getPosition();

	int left = pos.x - unit->getType().dimensionLeft();
	int right = pos.x + unit->getType().dimensionRight();
	int top = pos.y - unit->getType().dimensionUp();
	int bottom = pos.y + unit->getType().dimensionDown();

	int ratio_right = left + static_cast<int>((right - left) * ratio);
	int hp_top = top + yOffset + vertical_offset;
	int hp_bottom = top + 4 + yOffset + vertical_offset;

	BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hp_top), BWAPI::Position(right, hp_bottom), BWAPI::Colors::Grey,
	                            true);
	BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hp_top), BWAPI::Position(ratio_right, hp_bottom), color, true);
	BWAPI::Broodwar->drawBoxMap(BWAPI::Position(left, hp_top), BWAPI::Position(right, hp_bottom), BWAPI::Colors::Black,
	                            false);

	int tic_width = 3;

	for (int i(left); i < right - 1; i += tic_width)
	{
		BWAPI::Broodwar->drawLineMap(BWAPI::Position(i, hp_top), BWAPI::Position(i, hp_bottom), BWAPI::Colors::Black);
	}
}
