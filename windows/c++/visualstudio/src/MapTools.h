#pragma once

#include "Grid.hpp"

#include <BWAPI.h>
#include <vector>

#include "BWEM/src/bwem.h"
#include <iostream>

namespace MiraBot
{
	class MapTools
	{
		friend class Global;

		Grid<int> m_walkable; // whether a tile is buildable (includes static resources)          
		Grid<int> m_buildable; // whether a tile is buildable (includes static resources)
		Grid<int> m_depotBuildable;
		// whether a depot is buildable on a tile (illegal within 3 tiles of static resource)
		Grid<int> last_seen; // the last time any of our units has seen this position on the map
		int m_width = 0;
		int m_height = 0;
		int m_frame = 0;
		bool m_drawMap = false;

		static bool canBuild(int tile_x, int tile_y);
		static bool canWalk(int tile_x, int tile_y);
		void printMap() const;


	public:


		MapTools();

		BWEM::Map& map = BWEM::Map::Instance();
		const BWEM::Area* main_area = nullptr; // Main base area
		const BWEM::Area* snd_area = nullptr; // Second area (extension)
		std::vector<BWAPI::Position> getChokepoints(const BWEM::Area* area);


		void onStart();
		void onFrame();
		void draw() const;
		void toggleDraw();
		void onUnitDestroy(BWAPI::Unit unit) const;

		int width() const;
		int height() const;

		bool isValidTile(int tile_x, int tile_y) const;
		bool isValidTile(const BWAPI::TilePosition& tile) const;
		bool isValidPosition(const BWAPI::Position& pos) const;
		static bool isPowered(int tile_x, int tile_y);
		bool isExplored(int tile_x, int tile_y) const;
		bool isExplored(const BWAPI::Position& pos) const;
		bool isExplored(const BWAPI::TilePosition& pos) const;
		bool isVisible(int tile_x, int tile_y) const;
		bool isWalkable(int tile_x, int tile_y) const;
		bool isWalkable(const BWAPI::TilePosition& tile) const;
		bool isBuildable(int tile_x, int tileY) const;
		bool isBuildable(const BWAPI::TilePosition& tile) const;
		bool isDepotBuildableTile(int tile_x, int tile_y) const;
		static void drawTile(int tile_x, int tile_y, const BWAPI::Color& color);

		int MapTools::lastSeen(BWAPI::TilePosition pos);
		std::vector<BWEM::ChokePoint*> getChokePoints();
		BWAPI::Position getClosestCP(BWAPI::TilePosition tile_pos) const;
	};
}
