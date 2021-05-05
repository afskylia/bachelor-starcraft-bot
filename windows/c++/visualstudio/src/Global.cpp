#include "Global.h"

#include "ProductionManager.h"
#include "WorkerManager.h"
#include "CombatManager.h"
#include "MapTools.h"

using namespace MiraBot;

Global::Global()
{
	init();
}

Global& Global::Instance()
{
	static Global instance;
	return instance;
}

void Global::init()
{
	reset(m_mapTools);
	reset(m_productionManager);
	reset(m_workerManager);
	reset(m_combatManager);

}

void Global::GameStart()
{
	Instance().init();
}

MapTools& Global::Map() { return *get(Instance().m_mapTools); }
WorkerManager& Global::Workers() { return *get(Instance().m_workerManager); }
ProductionManager& Global::Production() { return *get(Instance().m_productionManager); }
CombatManager& Global::Combat() { return *get(Instance().m_combatManager); }