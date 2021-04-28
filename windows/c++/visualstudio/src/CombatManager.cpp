#include "CombatManager.h"

#include "MiraBot.h"

CombatManager::CombatManager() {
}

void CombatManager::onFrame() {
	rushEnemyBase();
}

void CombatManager::rushEnemyBase() {
	if (MiraBot::enemyStartLocation != BWAPI::TilePositions::None && MiraBot::m_zealot.size() > 10)
	{
		for (auto* zealot : MiraBot::m_zealot)
		{
			zealot->attack(BWAPI::Position(MiraBot::enemyStartLocation));
		}
		MiraBot::m_zealot.empty();
	}
}