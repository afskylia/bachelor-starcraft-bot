#include "CombatManager.h"

//#include "MiraBotMain.h"
#include "Global.h"
#include "Tools.h"
#include "CombatData.h"

using namespace MiraBot;

CombatManager::CombatManager()
{
}

void CombatManager::onFrame()
{
	if (m_offensive_units_.size() <= 10)
	{
		m_offensive_units_.attack(
			Tools::getClosestUnitTo(m_offensive_units_.getPosition(), Global::information().enemy_units));
	}
}

void CombatManager::onUnitComplete(BWAPI::Unit unit)
{
	// An attack unit
	for (auto protoss_attack_unit : m_combat_data_.protoss_attack_units)
	{
		if (unit->getType() == protoss_attack_unit)
		{
			addCombatUnit(unit);
			return;
		}
	}
}

void CombatManager::onUnitDestroy(BWAPI::Unit unit)
{
	auto it = m_attack_units_.find(unit);
	if (it != m_attack_units_.end()) m_attack_units_.erase(it);

	auto it_d = m_defensive_units_.find(unit);
	if (it_d != m_defensive_units_.end()) m_defensive_units_.erase(it_d);

	auto it_o = m_offensive_units_.find(unit);
	if (it_o != m_offensive_units_.end()) m_offensive_units_.erase(it_o);
}

// Send the unit to a chokepoint in the main base and stand ready
void CombatManager::guardBase(BWAPI::Unit unit)
{
}

void CombatManager::addCombatUnit(BWAPI::Unit unit)
{
	m_attack_units_.insert(unit);
	guardBase(unit);
}
