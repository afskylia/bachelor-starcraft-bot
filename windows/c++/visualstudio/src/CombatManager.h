#pragma once
#include "CombatData.h"

#include "Tools.h"
#include "Enums.h"


namespace MiraBot
{
	class CombatManager
	{
		friend class Global;
		CombatData m_combat_data_;

		BWAPI::Unitset m_defensive_units_ = BWAPI::Unitset::none;
		BWAPI::Unitset m_offensive_units_ = BWAPI::Unitset::none;


	public:
		CombatManager();
		void onFrame();
		void onUnitComplete(BWAPI::Unit unit);
		void onUnitShow(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitDestroy(BWAPI::Unit unit);

		void addCombatUnit(BWAPI::Unit unit);
		void resetTarget(BWAPI::Unit target);
		void resetCombatUnit(BWAPI::Unit unit);
		void handleIdleDefender(BWAPI::Unit unit);
		void handleIdleRetreater(BWAPI::Unit unit);
		void handleIdleAttacker(BWAPI::Unit unit);
		void handleIdleRallyer(BWAPI::Unit unit);
		void updateCombatStatus();
		void removeUnitTarget(BWAPI::Unit unit);


		void setTarget(BWAPI::Unit unit, BWAPI::Unit target); // Set unit's target unit
		void goRetreat(BWAPI::Unit unit);
		void goAttack(BWAPI::Unit unit);
		void goAttack(BWAPI::Unit unit, BWAPI::Position target_pos);
		void goRally(BWAPI::Unit unit);
		void goDefend(BWAPI::Unit unit);

		void updateAttackStatus();
		void cleanUpTargets();
		void startRushing();
		void retreatFromCombat();

		BWAPI::Position getChokepointToGuard(BWAPI::Unit unit); // Return chokepoint to be guarded by unit
		BWAPI::Unit chooseTarget(BWAPI::Unit unit, bool same_area = false); // Get closest target unit to unit

		bool attacking = false; // Whether we're currently rushing the enemy
		bool rallying = false;
		bool under_attack = false; // Whether we're currently under attack in one of our bases
		bool retreating = false;

		// Variables used for rushing
		//BWAPI::Position rush_target_pos = BWAPI::Positions::None;
		const BWEM::Area* rush_target; // Target area of current rush
		const BWEM::Area* rally_point; // Area in which to group up before rushing target
		int rallied_rushers; // Number of rushers who arrived at rally point
		int total_rusher_count = 0; // How many attack units we started rushing with in this rush
		int lost_rusher_count = 0; // How many of our attack units have died this rush
		void setRushTarget();

		// Set of all our attack units
		BWAPI::Unitset m_attack_units = BWAPI::Unitset::none;

		// Map of units and the position of the chokepoint the unit is defending
		std::map<BWAPI::Unit, BWAPI::Position> guard_map = {};

		// Set of targets (visible enemy units)
		BWAPI::Unitset targets;

		// Map of enemy targets and the number of our attack units that are targeting them
		std::map<BWAPI::Unit, int> target_attackers;

		// Map of our attack units and their status
		std::map<BWAPI::Unit, Enums::combat_status> fighter_status_map;

		// Map of our attack units and their current target
		std::map<BWAPI::Unit, BWAPI::Unit> fighter_target_map;
	};
}
