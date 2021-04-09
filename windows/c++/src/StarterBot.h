#pragma once

#include "MapTools.h"

#include <BWAPI.h>

class StarterBot
{
	BWAPI::Unit m_scout = nullptr;
    MapTools m_mapTools;
    /**
	 * \brief true if map is scouted
	 */
	bool m_scouted = false;
	//BWAPI::Race enemyRace = BWAPI::Races::None;
	bool foundEnemy = false;
	BWAPI::Race enemyRace = BWAPI::Races::None;
	BWAPI::TilePosition enemyStartLocation = BWAPI::TilePositions::None;
	//BWAPI::Unitset workers = BWAPI::Unitset::none;
	//BWAPI::Unitset attackUnits = BWAPI::Unitset::none;
	BWAPI::TilePosition mainBase = BWAPI::TilePositions::None;

public:

    StarterBot();

    // helper functions to get you started with bot programming and learn the API
    void sendIdleWorkersToMinerals();
    void trainAdditionalWorkers();
    void buildAdditionalSupply();
	void buildGateway();
	void buildAttackUnits();
    void drawDebugInformation();
    void sendScout();

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