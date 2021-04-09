#pragma once

#include <BWAPI.h>

namespace Tools
{
	BWAPI::Unit GetClosestUnitTo(BWAPI::Position p, const BWAPI::Unitset& units);
	BWAPI::Unit GetClosestUnitTo(BWAPI::Unit unit, const BWAPI::Unitset& units);

	int CountUnitsOfType(BWAPI::UnitType type, const BWAPI::Unitset& units = BWAPI::Broodwar->self()->getUnits());

	BWAPI::Unit GetUnitOfType(BWAPI::UnitType type);
	std::vector<BWAPI::Unit> GetUnitsOfType(BWAPI::UnitType type);
	BWAPI::Unit GetDepot();
	BWAPI::Unit GetWorker(BWAPI::UnitType unitType);

	bool BuildBuilding(BWAPI::UnitType type);

	void DrawUnitBoundingBoxes();
	void DrawUnitCommands();
	void DrawEnemyBases(BWAPI::TilePosition);

	void SmartRightClick(BWAPI::Unit unit, BWAPI::Unit target);

	int GetTotalSupply(bool inProgress = false);

	void DrawUnitHealthBars();
	void DrawHealthBar(BWAPI::Unit unit, double ratio, BWAPI::Color color, int yOffset);
}