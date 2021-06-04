#pragma once

#include "MapTools.h"

#include <BWAPI.h>


#include "ProductionManager.h"
#include "WorkerManager.h"
#include "CombatManager.h"
#include "StrategyManager.h"

namespace MiraBot
{
	class MiraBotMain
	{
		// Non-global managers can go here, e.g. a TimerManager

	public:
		MiraBotMain();

		// helper functions to get you started with bot programming and learn the API
		void drawDebugInformation();
		void logResult(bool is_winner);
		static std::string exec(const char* cmd);

		// functions that are triggered by various BWAPI events from main.cpp
		static void onStart();
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
