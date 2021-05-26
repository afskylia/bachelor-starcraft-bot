#pragma once
#include <BWAPI/Unit.h>

namespace MiraBot
{
	class InformationManager
	{
		friend class Global;


	public:
		static inline bool found_enemy = false;
		static inline BWAPI::Unit main_base = nullptr;
		static inline BWAPI::Race enemy_race = BWAPI::Races::None;
		static inline BWAPI::TilePosition enemy_start_location = BWAPI::TilePositions::None;
		static inline std::unordered_set<BWAPI::Unit> enemy_units = {};

		InformationManager();
		void logEnemyRaceAndStartLocation(BWAPI::Unit unit);
		void addOrRemoveEnemyUnit(BWAPI::Unit unit, bool remove_unit = false);
		void onUnitShow(BWAPI::Unit unit);
		void onStart();
		void onUnitDestroy(BWAPI::Unit unit);
	};
}
