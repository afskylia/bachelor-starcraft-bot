#include "MiraBotMain.h"


#include <fstream>
#include <iomanip>
#include <BWAPI/Client/Client.h>

#include "Tools.h"
#include "MapTools.h"
#include "Global.h"

using namespace MiraBot;

MiraBotMain::MiraBotMain() = default;

// Called when the bot starts!
void MiraBotMain::onStart()
{
	// Set our BWAPI options here    
	BWAPI::Broodwar->setLocalSpeed(0); // 10
	BWAPI::Broodwar->setFrameSkip(0);

	mainBase = BWAPI::Broodwar->getClosestUnit(BWAPI::Position(BWAPI::Broodwar->self()->getStartLocation()),
	                                           BWAPI::Filter::IsResourceDepot);

	// Enable the flag that tells BWAPI top let users enter input while bot plays
	BWAPI::Broodwar->enableFlag(BWAPI::Flag::UserInput);

	// Call MapTools OnStart
	Global::map().onStart();
}

// Win/lose, Timestamp, Length of game(frames), Enemy race, Own race, Map, Number of units built, Supply, Total supply, git message and branch
void MiraBotMain::logResult(bool is_winner)
{
	std::string whitespace = " \n\r\t\f\v";

	auto win = is_winner ? "win" : "loss";

	auto t = std::time(nullptr);
	auto tm = *std::localtime(&t);
	std::ostringstream oss;
	oss << std::put_time(&tm, "%d/%m/%Y");
	auto date = oss.str();
	oss.str("");
	oss << std::put_time(&tm, "%H.%M.%S");
	auto time = oss.str();


	auto frames = std::to_string(BWAPI::Broodwar->getFrameCount());
	auto race = BWAPI::Broodwar->self()->getRace().c_str();
	auto map = BWAPI::Broodwar->mapFileName();
	auto number_of_units = std::to_string(BWAPI::Broodwar->self()->allUnitCount());
	auto supply = std::to_string(BWAPI::Broodwar->self()->supplyUsed());
	auto total_supply = std::to_string(BWAPI::Broodwar->self()->supplyTotal());
	auto git = exec("git log -1 --pretty='%C(auto)%s'");
	size_t end = git.find_last_not_of(whitespace);
	git = git.substr(0, end + 1);
	auto branch = exec("git branch --show-current");
	end = branch.find_last_not_of(whitespace);
	branch = branch.substr(0, end + 1);

	std::ofstream file;
	file.open("log.csv", std::ios::in | std::ios::out | std::ios::ate);
	file << win << ';' << date << ';' << time << ';' << frames << ';' << enemyRace.toString() << ';' << race << ';' <<
		map << ';' << number_of_units << ';' << supply << ';' << total_supply << ';' << git << ';' << branch << "\n";
	file.close();
}

/**
 * Borrowed from https://stackoverflow.com/questions/478898/how-do-i-execute-a-command-and-get-the-output-of-the-command-within-c-using-po
 * execute cmd-commands and get result
 */
std::string MiraBotMain::exec(const char* cmd)
{
	std::array<char, 128> buffer;
	std::string result;
	std::unique_ptr<FILE, decltype(&_pclose)> pipe(_popen(cmd, "r"), _pclose);
	if (!pipe)
	{
		throw std::runtime_error("popen() failed!");
	}
	while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr)
	{
		result += buffer.data();
	}
	return result;
}

// Called whenever the game ends and tells you if you won or not
void MiraBotMain::onEnd(bool isWinner)
{
	logResult(isWinner);
	std::cout << "We " << (isWinner ? "won!" : "lost!") << "\n";
}

// Called on each frame of the game
void MiraBotMain::onFrame()
{
	// Managers on frame functions
	Global::workers().onFrame();
	Global::production().onFrame();
	Global::combat().onFrame();

	// Update our MapTools information
	Global::map().onFrame();


	// Draw unit health bars, which brood war unfortunately does not do
	Tools::DrawUnitHealthBars();

	// Draw some relevent information to the screen to help us debug the bot
	drawDebugInformation();
}


// Draw some relevant information to the screen to help us debug the bot
void MiraBotMain::drawDebugInformation()
{
	BWAPI::Broodwar->drawTextScreen(BWAPI::Position(10, 10), "Hello, World!\n");
	Tools::DrawUnitCommands();
	Tools::DrawUnitBoundingBoxes();

	if (foundEnemy) Tools::DrawEnemyBases(enemyStartLocation);
}

// Called whenever a unit is destroyed, with a pointer to the unit
void MiraBotMain::onUnitDestroy(BWAPI::Unit unit)
{
	if (unit->getType().isWorker()) Global::workers().onUnitDestroy(unit);
	// TODO maybe fix?
	Global::production().onUnitDestroy(unit);
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void MiraBotMain::onUnitMorph(BWAPI::Unit unit)
{
}

// Called whenever a text is sent to the game by a user
void MiraBotMain::onSendText(std::string text)
{
	if (text == "/map")
	{
		Global::map().toggleDraw();
	}
}

// Called whenever a unit is created, with a pointer to the unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void MiraBotMain::onUnitCreate(BWAPI::Unit unit)
{
	// TODO: Worker manager and combat manager have to decide which job to assign new units
	if (unit->getType().isWorker()) Global::workers().onUnitCreate(unit);
	// TODO: else combatmanager.onunitcreate()

	Global::production().onUnitComplete(unit);
}

// Called whenever a unit finished construction, with a pointer to the unit
void MiraBotMain::onUnitComplete(BWAPI::Unit unit)
{
	switch (unit->getType())
	{
	case BWAPI::UnitTypes::Protoss_Zealot:
		m_zealot.push_back(unit);
		break;
	default: break;
	}

	Global::production().onUnitComplete(unit);
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void MiraBotMain::onUnitShow(BWAPI::Unit unit)
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
void MiraBotMain::onUnitHide(BWAPI::Unit unit)
{
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void MiraBotMain::onUnitRenegade(BWAPI::Unit unit)
{
}
