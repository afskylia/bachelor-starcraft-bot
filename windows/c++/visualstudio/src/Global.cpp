#include "Global.h"

#include "ProductionManager.h"
#include "WorkerManager.h"
#include "CombatManager.h"
#include "MapTools.h"
#include "StrategyManager.h"

using namespace MiraBot;

Global::Global()
{
	init();
}

Global& Global::instance()
{
	static Global instance;
	return instance;
}

void Global::init()
{
	reset(m_map_tools_);
	reset(m_production_manager_);
	reset(m_worker_manager_);
	reset(m_combat_manager_);
	reset(m_strategy_manager_);
}

void Global::gameStart()
{
	instance().init();
}

MapTools& Global::map() { return *get(instance().m_map_tools_); }
WorkerManager& Global::workers() { return *get(instance().m_worker_manager_); }
ProductionManager& Global::production() { return *get(instance().m_production_manager_); }
CombatManager& Global::combat() { return *get(instance().m_combat_manager_); }
StrategyManager& Global::strategy() { return *get(instance().m_strategy_manager_); }
