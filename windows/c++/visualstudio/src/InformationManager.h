﻿#pragma once
#include <BWAPI/Unit.h>
#include "Enums.h"

namespace MiraBot
{
	class InformationManager
	{
		friend class Global;

		bool m_should_update_ = false;

	public:
		bool found_enemy = false;
		Enums::strategy_type m_current_enemy_strategy_ = Enums::strategy_type::none;
		BWAPI::Unit main_base = nullptr;
		BWAPI::Race enemy_race = BWAPI::Races::None;
		BWAPI::TilePosition enemy_start_location = BWAPI::TilePositions::None;
		std::unordered_set<BWAPI::Unit> enemy_units = {};

		InformationManager();
		void updateEnemyStrategy();
		void informationIsUpdated();
		void onFrame();
		void informationUpdateShouldHappen();
		void logEnemyRaceAndStartLocation(BWAPI::Unit unit);
		void addOrRemoveEnemyUnit(BWAPI::Unit unit, bool remove_unit = false);
		void onUnitShow(BWAPI::Unit unit);
		void onStart();
		void onUnitDestroy(BWAPI::Unit unit);
		Enums::strategy_type getEnemyStrategy();
	};
}
