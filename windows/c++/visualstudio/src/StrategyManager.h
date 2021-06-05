#pragma once
#include <map>
#include <BWAPI/Race.h>

#include "BuildOrderData.h"
#include "Enums.h"

namespace MiraBot
{
	class StrategyManager
	{
		friend class Global;


	public:
		BuildOrderData m_build_order_data;
		std::map<int, BWAPI::UnitType> m_build_order;
		int prev_supply{};

		std::map<int, BWAPI::UnitType> getBuildOrder(BWAPI::Race enemy_race = BWAPI::Races::None,
		                                             Enums::strategy_type enemy_strategy = Enums::offensive);

		StrategyManager();
		void onFrame();
		void switchBuildOrder(std::map<int, BWAPI::UnitType> new_build_order);
		void informationUpdate();
	};
}
