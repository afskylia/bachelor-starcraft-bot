#include "MiraBotMain.h"


#include <fstream>
#include <iomanip>
#include <BWAPI/Client/Client.h>

#include "Tools.h"
#include "MapTools.h"
#include "Global.h"

#include "BWEM/src/bwem.h"
#include <iostream>

using namespace MiraBot;
using namespace BWAPI;
using namespace Filter;

//using namespace BWEM;
//using namespace BWEM::BWAPI_ext;
//using namespace BWEM::utils;


MiraBotMain::MiraBotMain() = default;

// Called when the bot starts!
void MiraBotMain::onStart()
{
	try
	{
		// Set initial local game speed
		Broodwar->setLocalSpeed(10); // 10
		Broodwar->setFrameSkip(0);

		// Enable the flag that tells BWAPI to let users enter input while bot plays
		Broodwar->enableFlag(Flag::UserInput);

		// Call MapTools OnStart
		Global::map().onStart();
		Global::information().onStart();
	}
	catch (const std::exception& e)
	{
		std::cout << "EXCEPTION: " << e.what() << std::endl;
	}
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


	auto frames = std::to_string(Broodwar->getFrameCount());
	auto race = Broodwar->self()->getRace().c_str();
	auto map = Broodwar->mapFileName();
	auto number_of_units = std::to_string(Broodwar->self()->allUnitCount());
	auto supply = std::to_string(Broodwar->self()->supplyUsed());
	auto total_supply = std::to_string(Broodwar->self()->supplyTotal());
	auto git = exec("git log -1 --pretty='%C(auto)%s'");
	size_t end = git.find_last_not_of(whitespace);
	git = git.substr(0, end + 1);
	auto branch = exec("git branch --show-current");
	end = branch.find_last_not_of(whitespace);
	branch = branch.substr(0, end + 1);

	std::ofstream file;
	file.open("log.csv", std::ios::in | std::ios::out | std::ios::ate);
	file << win << ';' << date << ';' << time << ';' << frames << ';' << Global::information().enemy_race.toString() <<
		';' << race << ';' <<
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
	Global::strategy().onFrame();
	Global::information().onFrame();

	// Update our MapTools information & draw
	Global::map().onFrame();


	// Draw unit health bars, which brood war unfortunately does not do
	Tools::drawUnitHealthBars();

	// Draw some relevent information to the screen to help us debug the bot
	drawDebugInformation();
}


// Draw some relevant information to the screen to help us debug the bot
void MiraBotMain::drawDebugInformation()
{
	Tools::drawUnitCommands();
	Tools::drawUnitBoundingBoxes();

	if (Global::information().found_enemy) Tools::drawEnemyBases(Global::information().enemy_start_location);
}

// Called whenever a unit is destroyed, with a pointer to the unit
void MiraBotMain::onUnitDestroy(Unit unit)
{
	// BWEM updates
	Global::map().onUnitDestroy(unit);

	// TODO maybe fix?
	if (unit->getType().isWorker()) Global::workers().onUnitDestroy(unit);
	Global::production().onUnitDestroy(unit);
	Global::information().onUnitDestroy(unit);
	Global::combat().onUnitDestroy(unit);
}

// Called whenever a unit is morphed, with a pointer to the unit
// Zerg units morph when they turn into other units
void MiraBotMain::onUnitMorph(Unit unit)
{
}

// Called whenever a text is sent to the game by a user
void MiraBotMain::onSendText(std::string text)
{
	if (text == "speed0") Broodwar->setLocalSpeed(25); // Slowest speed
	else if (text == "speed1") Broodwar->setLocalSpeed(17);
	else if (text == "speed2") Broodwar->setLocalSpeed(10);
	else if (text == "speed3") Broodwar->setLocalSpeed(0); // Fastest speed
		//else if (text == "/map")Global::map().toggleDraw();

	else if (text == "production") Global::production().printDebugData();

		// TODO: Why doesn't this work????
	else BWEM::utils::MapDrawer::ProcessCommand(text);
}

// Called whenever a unit is created, with a pointer to the unit
// Units are created in buildings like barracks before they are visible, 
// so this will trigger when you issue the build command for most units
void MiraBotMain::onUnitCreate(Unit unit)
{
	// TODO: Worker manager and combat manager have to decide which job to assign new units
	if (unit->getType().isWorker()) Global::workers().onUnitCreate(unit);
	// TODO: else combatmanager.onunitcreate()

	Global::production().onUnitComplete(unit);
}

// Called whenever a unit finished construction, with a pointer to the unit
void MiraBotMain::onUnitComplete(Unit unit)
{
	Global::production().onUnitComplete(unit);
	Global::combat().onUnitComplete(unit);
}

// Called whenever a unit appears, with a pointer to the destroyed unit
// This is usually triggered when units appear from fog of war and become visible
void MiraBotMain::onUnitShow(Unit unit)
{
	Global::information().onUnitShow(unit);
}

// Called whenever a unit gets hidden, with a pointer to the destroyed unit
// This is usually triggered when units enter the fog of war and are no longer visible
void MiraBotMain::onUnitHide(Unit unit)
{
}

// Called whenever a unit switches player control
// This usually happens when a dark archon takes control of a unit
void MiraBotMain::onUnitRenegade(Unit unit)
{
}
