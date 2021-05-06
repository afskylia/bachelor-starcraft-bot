#pragma once

#include "MapTools.h"

#include <BWAPI.h>


#include "ProductionManager.h"
#include "WorkerManager.h"
#include "CombatManager.h"

namespace MiraBot
{


	class MiraBotMain
	{
		// Non-global managers can go here, e.g. a TimerManager

		// Managers
		//MapTools m_mapTools;
		//ProductionManager m_production_manager;
		//CombatManager m_combat_manager;
		//WorkerManager m_worker_manager;
		bool foundEnemy = false;

		//BWAPI::Unitset workers = BWAPI::Unitset::none;
		//BWAPI::Unitset attackUnits = BWAPI::Unitset::none;

		int m_zealot_count = 0;

	public:


		static inline BWAPI::Unit m_scout = nullptr;
		static inline BWAPI::Unit mainBase = nullptr;
		static inline BWAPI::Race enemyRace = BWAPI::Races::None;
		static inline BWAPI::TilePosition enemyStartLocation = BWAPI::TilePositions::None;

		/**
		* \brief rushing zealots
		*/
		static inline std::vector<BWAPI::Unit> m_zealot;

		MiraBotMain();

		// helper functions to get you started with bot programming and learn the API
		void drawDebugInformation();

		// functions that are triggered by various BWAPI events from main.cpp
		void onStart();
		void onFrame();
		void onEnd(bool isWinner);
		void onUnitDestroy(BWAPI::Unit unit);
		void onUnitMorph(BWAPI::Unit unit);
		void onSendText(std::string text);
		void onUnitCreate(BWAPI::Unit unit);
		void onUnitComplete(BWAPI::Unit unit);
		void onUnitShow(BWAPI::Unit unit);
		void onUnitHide(BWAPI::Unit unit);
		void onUnitRenegade(BWAPI::Unit unit);
	};
}