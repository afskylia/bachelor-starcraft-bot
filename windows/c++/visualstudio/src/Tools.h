#pragma once

#include <BWAPI.h>

#include "WorkerManager.h"
#include "MiraBotMain.h"

namespace Tools
{
	std::vector<BWAPI::Unit> sortUnitsByClosest(BWAPI::Unit unit, const BWAPI::Unitset& units);
	BWAPI::Unit getClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units);
	BWAPI::Unit getClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units);

	int countUnitsOfType(BWAPI::UnitType type, bool completed = true, bool idle = false,
	                     BWAPI::Unitset units = BWAPI::Broodwar->self()->getUnits());

	BWAPI::Unit GetWorker(BWAPI::UnitType unitType, BWAPI::Position building_position);

	BWAPI::Unit getUnitOfType(BWAPI::UnitType type);
	std::vector<BWAPI::Unit> getUnitsOfType(BWAPI::UnitType type);
	BWAPI::Unit getDepot();
	BWAPI::Unit GetWorker(BWAPI::UnitType unitType);

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
