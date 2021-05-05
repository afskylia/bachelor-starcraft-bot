#include "CombatManager.h"

#include "MiraBotMain.h"
#include "Global.h"

using namespace MiraBot;

CombatManager::CombatManager() {
}

void CombatManager::onFrame() {
	rushEnemyBase();
}

void CombatManager::rushEnemyBase() {
	if (MiraBotMain::enemyStartLocation != BWAPI::TilePositions::None && MiraBotMain::m_zealot.size() > 10)
	{
		for (auto* zealot : MiraBotMain::m_zealot)
		{
			zealot->attack(BWAPI::Position(MiraBotMain::enemyStartLocation));
		}
		MiraBotMain::m_zealot.empty();
	}
}