#include "StrategyManager.h"
#include "MiraBotMain.h"
#include "Global.h"
#include "BuildOrderData.h"

using namespace MiraBot;


/// <summary>
/// Gets new build order from data. TODO enemy strat not used
/// </summary>
/// <param name="race"></param>
/// <param name="enemy_race"></param>
/// <param name="enemy_strategy"></param>
/// <returns></returns>
std::map<int, std::pair<BWAPI::UnitType, int>> StrategyManager::getBuildOrder(BWAPI::Race enemy_race,
                                                                              Enums::strategy_type enemy_strategy)
{
	switch (enemy_race)
	{
	case BWAPI::Races::None:
		return m_build_order_data.starter_build_order;
	case BWAPI::Races::Terran:
		return m_build_order_data.protoss_v_terran;
	case BWAPI::Races::Protoss:
		return m_build_order_data.protoss_v_protoss;
	case BWAPI::Races::Zerg:
		return m_build_order_data.protoss_v_zerg;
	default: return m_build_order_data.starter_build_order;
	}
}

StrategyManager::StrategyManager()
{
	m_build_order = getBuildOrder();
};

void StrategyManager::onFrame()
{
}

/// <summary>
/// Called when information manager has new information e.g. enemy race
/// </summary>
void StrategyManager::informationUpdate()
{
	//std::cout << "Information is updated \n";
	// do not update if supply > 20
	if (BWAPI::Broodwar->self()->supplyUsed() <= 20)
	{
		m_build_order = getBuildOrder(Global::information().enemy_race,
		                              Global::information().m_current_enemy_strategy);
		Global::production().printDebugData();
		// TODO: Update production (build queue, prev_supply and enqueued_items here!!)
		// compare old build order to new one and set prev_supply, enqueued_items etc.
	}
}

bool StrategyManager::shouldStartRushing()
{
	if (Global::combat().under_attack) return false; // Don't start rushing while we ourselves are under attack
	// TODO only rush if offensive strategy?

	auto enemy_units = Global::information().enemy_units;

	if (enemy_units.empty()) return false;

	std::vector<BWAPI::Unit> enemy_attack_units = {};

	for (auto* u : enemy_units)
	{
		if (!u->getType().isWorker() && u->canAttack())
			enemy_attack_units.push_back(u);
	}

	const auto our_attack_units = Global::combat().m_attack_units_;

	if (our_attack_units.size() >= 20 && enemy_attack_units.size() < our_attack_units.size())
		return true;

	return false;
}
