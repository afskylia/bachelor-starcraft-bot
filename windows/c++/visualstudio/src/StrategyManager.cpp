#include "StrategyManager.h"
#include "MiraBotMain.h"
#include "Global.h"
#include "BuildOrderData.h"

using namespace MiraBot;


/// <summary>
/// Gets new build order from data
/// TODO consider more variables
/// </summary>
/// <param name="race"></param>
/// <param name="enemy_race"></param>
/// <param name="enemy_strategy"></param>
/// <returns></returns>
std::map<int, BWAPI::UnitType> StrategyManager::getBuildOrder(BWAPI::Race enemy_race,
                                                              strategy_type enemy_strategy)
{
	std::cout << "Updating Build Order \n";
	switch (enemy_race)
	{
	case BWAPI::Races::None:
		return BuildOrderData::starter_build_order;
	case BWAPI::Races::Terran:
		return BuildOrderData::protoss_v_terran;
	default: return BuildOrderData::starter_build_order;
	}
}

StrategyManager::StrategyManager()
{
	m_build_order_ = getBuildOrder();
};

void StrategyManager::onFrame()
{
}

/// <summary>
/// Called when information manager has new information e.g. enemy race
/// </summary>
void StrategyManager::informationUpdate()
{
	std::cout << "Information is updated \n";
	m_build_order_ = getBuildOrder(InformationManager::enemy_race,
	                               Global::information().getEnemyStrategy());
}
