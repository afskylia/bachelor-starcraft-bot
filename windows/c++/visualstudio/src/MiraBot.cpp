#include "MiraBot.h"

#include <BWAPI/Client/Client.h>

#include "Tools.h"
#include "MapTools.h"

MiraBot::MiraBot()
{
}

// Called when the bot starts!
void MiraBot::onStart()
{
	// Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(10);
	BWAPI::Broodwar->setFrameSkip(0);

	mainBase = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()), BWAPI::Filter::IsResourceDepot);

	// Enable the flag that tells BWAPI top let users enter input while bot plays
	BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

	// Call MapTools OnStart
	m_mapTools.onStart();
}

// Called whenever the game ends and tells you if you won or not
void MiraBot::onEnd(bool isWinner)
{
	std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called on each frame of the game
void MiraBot::onFrame()
{
	// Managers on frame functions
	m_worker_manager.onFrame();
	m_production_manager.onFrame();
	m_combat_manager.onFrame();
	// Update our MapTools information
	m_mapTools.onFrame();

	// Draw unit health bars, which brood war unfortunately does not do
	Tools::DrawUnitHealthBars();

	// Draw some relevent information to the screen to help us debug the bot
	drawDebugInformation();
}


// Draw some relevant information to the screen to help us debug the bot
void MiraBot::drawDebugInformation()
{
	BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
	Tools::DrawUnitCommands();
	Tools::DrawUnitBoundingBoxes();

	if (foundEnemy) Tools::DrawEnemyBases(enemyStartLocation);
}

// Called whenever a unit is destroyed, with a pointer to the unit
void MiraBot::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isWorker()) m_worker_manager.onUnitDestroy(unit);
	m_production_manager.onUnitDestroy(unit);
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void MiraBot::onUnitMorph(BWAPI::Unit unit)
{
}

// Called whenever a text is sent to the game by a user
void MiraBot::onSendText(std::string text)
{
	if (text == "/map")
	{
		m_mapTools.toggleDraw();
	}
}

// Called whenever a unit is created, with a pointer to the unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void MiraBot::onUnitCreate(BWAPI::Unit unit)
{
	// TODO: Worker manager and combat manager have to decide which job to assign new units
	if (unit->getType().isWorker()) m_worker_manager.onUnitCreate(unit);
	// TODO: else combatmanager.onunitcreate()

	//m_production_manager.onUnitComplete(unit);
}

// Called whenever a unit finished construction, with a pointer to the unit
void MiraBot::onUnitComplete(BWAPI::Unit unit)
{
	switch (unit->getType())
	{
	case BWAPI::UnitTypes::Protoss_Zealot:
		m_zealot.push_back(unit);
		break;
	default: break;
	}

	m_production_manager.onUnitComplete(unit);
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void MiraBot::onUnitShow(BWAPI::Unit unit)
{
	auto* unitPlayer = unit->getPlayer();
	if (!foundEnemy && unitPlayer->isEnemy(BWAPI::Broodwar->self()))
	{
		enemyRace = unit->getType().getRace();
		foundEnemy = true;
		std::cout << "Enemy is " << enemyRace << "\n";

		auto& startLocations = BWAPI::Broodwar->getStartLocations();

		// Find closest starting location to enemy unit
		// TODO: Also save locations of other enemy bases they might build later in the game
		double shortestDistance = INT_MAX;
		for (BWAPI::TilePosition position : startLocations)
		{
			const auto distance = position.getDistance(unit->getTilePosition());
			if (distance < shortestDistance)
			{
				shortestDistance = distance;
				enemyStartLocation = position;
			}
		}
		std::cout << "Enemy starting location: " << enemyStartLocation << "\n";
	}
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void MiraBot::onUnitHide(BWAPI::Unit unit)
{
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void MiraBot::onUnitRenegade(BWAPI::Unit unit)
{
}
