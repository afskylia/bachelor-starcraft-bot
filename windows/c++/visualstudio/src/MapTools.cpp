#include "MapTools.h"

#include <iostream>
#include <sstream>
#include <fstream>
#include <array>
#include "Global.h"

#include "BWEM/src/bwem.h"
#include <iostream>

using namespace MiraBot;
using namespace BWEM;


MapTools::MapTools() = default;


void MapTools::onStart()
{
	// Initialize BWEM
	std::cout << "Map initialization...";
	map.Initialize();
	map.EnableAutomaticPathAnalysis();
	const auto starting_locations_ok = map.FindBasesForStartingLocations();
	assert(starting_locations_ok);

	utils::MapPrinter::Initialize(&map);

	utils::printMap(map); // will print the map into the file bin/map.bmp
	utils::pathExample(map); // add to the printed map a path between two starting locations

	std::cout << " complete!\n";


	// Initialize map grid stuff
	m_width = BWAPI::Broodwar->mapWidth();
	m_height = BWAPI::Broodwar->mapHeight();
	m_walkable = Grid<int>(m_width, m_height, 1);
	m_buildable = Grid<int>(m_width, m_height, 0);
	m_depotBuildable = Grid<int>(m_width, m_height, 0);
	m_lastSeen = Grid<int>(m_width, m_height, 0);

	// Set the boolean grid data from the Map
	for (auto x(0); x < m_width; ++x)
	{
		for (auto y(0); y < m_height; ++y)
		{
			m_buildable.set(x, y, canBuild(x, y));
			m_depotBuildable.set(x, y, canBuild(x, y));
			m_walkable.set(x, y, m_buildable.get(x, y) || canWalk(x, y));
		}
	}

	// set tiles that static resources are on as unbuildable
	for (auto& resource : BWAPI::Broodwar->getStaticNeutralUnits())
	{
		if (!resource->getType().isResourceContainer())
		{
			continue;
		}

		const auto tile_x = resource->getTilePosition().x;
		const auto tile_y = resource->getTilePosition().y;

		for (auto x = tile_x; x < tile_x + resource->getType().tileWidth(); ++x)
		{
			for (auto y = tile_y; y < tile_y + resource->getType().tileHeight(); ++y)
			{
				m_buildable.set(x, y, false);

				// depots can't be built within 3 tiles of any resource
				for (auto rx = -3; rx <= 3; rx++)
				{
					for (auto ry = -3; ry <= 3; ry++)
					{
						if (!BWAPI::TilePosition(x + rx, y + ry).isValid())
						{
							continue;
						}

						m_depotBuildable.set(x + rx, y + ry, 0);
					}
				}
			}
		}
	}
}

void MapTools::onFrame()
{
	// Update last-seen grid
	for (auto x = 0; x < m_width; ++x)
	{
		for (auto y = 0; y < m_height; ++y)
		{
			if (isVisible(x, y))
				m_lastSeen.set(x, y, BWAPI::Broodwar->getFrameCount());
		}
	}

	// Draw stuff such as health bars
	if (m_drawMap) draw();

	// BWEM drawing functions are super slow in debug mode
#ifdef NDEBUG
	utils::gridMapExample(map);
	utils::drawMap(map);
#endif
}

void MapTools::toggleDraw()
{
	m_drawMap = !m_drawMap;
}

void MapTools::onUnitDestroy(const BWAPI::Unit unit) const
{
	if (unit->getType().isMineralField()) map.OnMineralDestroyed(unit);
	else if (unit->getType().isSpecialBuilding()) map.OnStaticBuildingDestroyed(unit);
}

bool MapTools::isExplored(const BWAPI::TilePosition& pos) const
{
	return isExplored(pos.x, pos.y);
}

bool MapTools::isExplored(const BWAPI::Position& pos) const
{
	return isExplored(BWAPI::TilePosition(pos));
}

bool MapTools::isExplored(const int tile_x, const int tile_y) const
{
	if (!isValidTile(tile_x, tile_y)) { return false; }

	return BWAPI::Broodwar->isExplored(tile_x, tile_y);
}

bool MapTools::isVisible(const int tile_x, const int tile_y) const
{
	if (!isValidTile(tile_x, tile_y)) { return false; }

	return BWAPI::Broodwar->isVisible(BWAPI::TilePosition(tile_x, tile_y));
}

bool MapTools::isPowered(const int tile_x, const int tile_y)
{
	return BWAPI::Broodwar->hasPower(BWAPI::TilePosition(tile_x, tile_y));
}

bool MapTools::isValidTile(const int tile_x, const int tile_y) const
{
	return tile_x >= 0 && tile_y >= 0 && tile_x < m_width && tile_y < m_height;
}

bool MapTools::isValidTile(const BWAPI::TilePosition& tile) const
{
	return isValidTile(tile.x, tile.y);
}

bool MapTools::isValidPosition(const BWAPI::Position& pos) const
{
	return isValidTile(BWAPI::TilePosition(pos));
}

bool MapTools::isBuildable(const int tile_x, const int tileY) const
{
	if (!isValidTile(tile_x, tileY))
		return false;

	return m_buildable.get(tile_x, tileY);
}

bool MapTools::isBuildable(const BWAPI::TilePosition& tile) const
{
	return isBuildable(tile.x, tile.y);
}

void MapTools::printMap() const
{
	std::stringstream ss;
	for (auto y(0); y < m_height; ++y)
	{
		for (auto x(0); x < m_width; ++x)
			ss << isWalkable(x, y);

		ss << "\n";
	}

	std::ofstream out("map.txt");
	out << ss.str();
	out.close();
}

bool MapTools::isDepotBuildableTile(const int tile_x, const int tile_y) const
{
	if (!isValidTile(tile_x, tile_y))
		return false;

	return m_depotBuildable.get(tile_x, tile_y);
}

bool MapTools::isWalkable(const int tile_x, const int tile_y) const
{
	if (!isValidTile(tile_x, tile_y))
	{
		return false;
	}

	return m_walkable.get(tile_x, tile_y);
}

bool MapTools::isWalkable(const BWAPI::TilePosition& tile) const
{
	return isWalkable(tile.x, tile.y);
}

int MapTools::width() const
{
	return m_width;
}

int MapTools::height() const
{
	return m_height;
}

void MapTools::drawTile(const int tile_x, const int tile_y, const BWAPI::Color& color)
{
	const auto padding = 2;
	const auto px = tile_x * 32 + padding;
	const auto py = tile_y * 32 + padding;
	const auto d = 32 - 2 * padding;

	BWAPI::Broodwar->drawLineMap(px, py, px + d, py, color);
	BWAPI::Broodwar->drawLineMap(px + d, py, px + d, py + d, color);
	BWAPI::Broodwar->drawLineMap(px + d, py + d, px, py + d, color);
	BWAPI::Broodwar->drawLineMap(px, py + d, px, py, color);
}

// Returns the position of the closest chokepoint to given tile position
BWAPI::Position MapTools::getClosestCP(const BWAPI::TilePosition tile_pos) const
{
	// Cast to position
	const auto pos = BWAPI::Position(tile_pos);

	// Find the position of the closest chokepoint
	auto closest_cp = BWAPI::Positions::None;
	const auto* area = map.GetNearestArea(tile_pos);
	auto chokepoints = area->ChokePoints();
	for (const auto* cp : chokepoints)
	{
		auto cp_pos = BWAPI::Position(cp->Center());
		closest_cp = cp_pos;
		if (closest_cp == BWAPI::Positions::None || closest_cp.getDistance(pos) > cp_pos.getDistance(pos))
		{
			closest_cp = cp_pos;
		}
	}

	return closest_cp;
}

bool MapTools::canWalk(const int tile_x, const int tile_y)
{
	for (auto i = 0; i < 4; ++i)
	{
		for (auto j = 0; j < 4; ++j)
		{
			if (!BWAPI::Broodwar->isWalkable(tile_x * 4 + i, tile_y * 4 + j))
			{
				return false;
			}
		}
	}

	return true;
}

bool MapTools::canBuild(const int tile_x, const int tile_y)
{
	return BWAPI::Broodwar->isBuildable(BWAPI::TilePosition(tile_x, tile_y));
}

void MapTools::draw() const
{
	const BWAPI::TilePosition screen(BWAPI::Broodwar->getScreenPosition());
	const auto sx = screen.x;
	const auto sy = screen.y;
	const auto ex = sx + 20;
	const auto ey = sy + 15;

	for (auto x = sx; x < ex; ++x)
	{
		for (auto y = sy; y < ey; y++)
		{
			const BWAPI::TilePosition tile_pos(x, y);
			if (!tile_pos.isValid()) { continue; }

			if constexpr (true)
			{
				auto color = isWalkable(x, y) ? BWAPI::Color(0, 255, 0) : BWAPI::Color(255, 0, 0);
				if (isWalkable(x, y) && !isBuildable(x, y)) { color = BWAPI::Color(255, 255, 0); }
				if (isBuildable(x, y) && !isDepotBuildableTile(x, y)) { color = BWAPI::Color(127, 255, 255); }
				drawTile(x, y, color);
			}
		}
	}

	const auto red = '\x08';
	const auto green = '\x07';
	const auto white = '\x04';
	const auto yellow = '\x03';

	BWAPI::Broodwar->drawBoxScreen(0, 0, 200, 100, BWAPI::Colors::Black, true);
	BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Huge);
	BWAPI::Broodwar->drawTextScreen(10, 5, "%cMap Legend", white);
	BWAPI::Broodwar->setTextSize(BWAPI::Text::Size::Default);
	BWAPI::Broodwar->drawTextScreen(10, 30, "%cRed: ", red);
	BWAPI::Broodwar->drawTextScreen(60, 30, "%cCan't Walk or Build", white);
	BWAPI::Broodwar->drawTextScreen(10, 45, "%cGreen:", green);
	BWAPI::Broodwar->drawTextScreen(60, 45, "%cCan Walk and Build", white);
	BWAPI::Broodwar->drawTextScreen(10, 60, "%cYellow:", yellow);
	BWAPI::Broodwar->drawTextScreen(60, 60, "%cResource Tile, Can't Build", white);
	BWAPI::Broodwar->drawTextScreen(10, 75, "Teal:");
	BWAPI::Broodwar->drawTextScreen(60, 75, "%cCan't Build Depot", white);
}
