#pragma once
#include <map>
#include <BWAPI/Race.h>

#include "BuildOrderData.h"

namespace MiraBot
{
	class StrategyManager
	{
		friend class Global;


	public:
		BuildOrderData m_build_order_data;
		std::map<int, BWAPI::UnitType> m_build_order;

		//enum strategy_type { offensive = 0, defensive = 1, expanding = 2, none = 3 };


		std::map<int, BWAPI::UnitType> getBuildOrder(BWAPI::Race enemy_race = BWAPI::Races::None,
		                                             Enums::strategy_type enemy_strategy = Enums::offensive);

		StrategyManager();
		void onFrame();
		void informationUpdate();
	};
}
