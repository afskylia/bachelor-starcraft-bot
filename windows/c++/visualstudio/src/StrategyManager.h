#pragma once
#include <map>
#include <BWAPI/Race.h>

namespace BWAPI
{
	class UnitType;
}

namespace MiraBot
{
	class StrategyManager
	{
		friend class Global;


	public:
		static inline std::map<int, BWAPI::UnitType> m_build_order_;

		enum strategy_type { offensive = 0, defensive = 1, expanding = 2, none = 3 };


		std::map<int, BWAPI::UnitType> getBuildOrder(BWAPI::Race enemy_race = BWAPI::Races::None,
		                                             strategy_type enemy_strategy = offensive);

		StrategyManager();
		void onFrame();
		void informationUpdate();
	};
}
