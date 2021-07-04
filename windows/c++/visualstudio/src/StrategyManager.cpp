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
std::map<int, std::pair<BWAPI::UnitType, int>> StrategyManager::setBuildOrder(BWAPI::Race enemy_race,
                                                                              Enums::strategy_type enemy_strategy)
{
	auto prev_build_order = m_build_order;
	auto new_build_order = prev_build_order;

	switch (enemy_race)
	{
	case BWAPI::Races::None:
		{
			new_build_order = m_build_order_data.starter_build_order;
			break;
		}
	case BWAPI::Races::Terran:
		{
			new_build_order = m_build_order_data.protoss_v_terran;
			break;
		}
	case BWAPI::Races::Protoss:
		{
			new_build_order = m_build_order_data.protoss_v_protoss;
			break;
		}
	case BWAPI::Races::Zerg:
		{
			new_build_order = m_build_order_data.protoss_v_zerg;
			break;
		}
	}
	std::vector<int> new_enqueued_levels = {4};
	if (prev_build_order != new_build_order)
	{
		std::cout << "ENQUEUED LEVELS: ";
		for (auto p : Global::production().enqueued_levels)
		{
			std::cout << p << " ";
		}
		std::cout << "\n";

		std::cout << "OLD BUILD ORDER: ";
		for (auto p : prev_build_order)
		{
			std::cout << "(" << p.first << ", " << p.second.first << ") ";
		}

		std::cout << "\nNEW BUILD ORDER: ";
		for (auto p : new_build_order)
		{
			std::cout << "(" << p.first << ", " << p.second.first << ") ";
		}
		std::cout << "\n";

		// Reset build order information in production manager
		auto enqueued_levels = Global::production().enqueued_levels;
		for (auto [lvl, pair] : prev_build_order)
		{
			if (new_build_order[lvl].first == pair.first)
			{
				if (std::count(enqueued_levels.begin(), enqueued_levels.end(), lvl))
				{
					new_enqueued_levels.push_back(lvl);
					Global::production().prev_supply = lvl;
				}
				else break;
			}
		}
	}

	Global::production().enqueued_levels = new_enqueued_levels;
	m_build_order = new_build_order;
	return new_build_order;
}

StrategyManager::StrategyManager()
{
	setBuildOrder();
};

void StrategyManager::onFrame()
{
}

/// <summary>
/// Called when information manager has new information e.g. enemy race
/// </summary>
void StrategyManager::informationUpdate()
{
	// Do not update if supply > 20
	if (BWAPI::Broodwar->self()->supplyUsed() <= 20)
	{
		// Update build order
		setBuildOrder(Global::information().enemy_race,
		              Global::information().m_current_enemy_strategy);
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

	const auto our_attack_units = Global::combat().m_attack_units;

	if (our_attack_units.size() >= 20 && enemy_attack_units.size() < our_attack_units.size())
		return true;

	return false;
}
