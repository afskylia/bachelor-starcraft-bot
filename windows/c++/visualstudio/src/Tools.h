#pragma once

#include <BWAPI.h>

#include "WorkerManager.h"
#include "MiraBotMain.h"

namespace Tools
{
	std::vector<BWAPI::Unit> sortUnitsByClosest(BWAPI::Position pos, const BWAPI::Unitset& units);
	std::vector<BWAPI::Unit> sortUnitsByClosest(BWAPI::Unit unit, const BWAPI::Unitset& units);

	BWAPI::Unit getClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units);
	BWAPI::Unit getClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units);

	int countUnitsOfType(BWAPI::UnitType type, bool completed = true, bool idle = false,
	                     BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits());

	BWAPI::Unit GetWorker(BWAPI::UnitType unitType, BWAPI::Position building_position);

	BWAPI::Unit getUnitOfType(BWAPI::UnitType type, bool idle = false, bool completed = true);
	std::vector<BWAPI::Unit> getUnitsOfType(BWAPI::UnitType type, bool idle = false, bool completed = true);

	BWAPI::Unit getDepot();

	void drawUnitBoundingBoxes();
	void drawUnitCommands();
	void drawEnemyBases(BWAPI::TilePosition);

	void smartRightClick(BWAPI::Unit unit, BWAPI::Unit target);

	int getTotalSupply(bool in_progress = false);
	int getTotalUsedSupply(bool in_progress_and_building_supply = false);

	void drawUnitHealthBars();
	void drawHealthBar(BWAPI::Unit unit, double ratio, BWAPI::Color color, int yOffset);
	void drawPoint(BWAPI::Position);
}
