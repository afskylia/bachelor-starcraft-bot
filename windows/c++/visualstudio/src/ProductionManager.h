#pragma once
#include <iostream>
#include <BWAPI/Unit.h>

namespace MiraBot
{


	class ProductionManager
	{
		friend class Global;
		
	public:

		ProductionManager();
		void onFrame();
		void buildGateway();
		void buildAttackUnits();
		void buildAdditionalSupply();

		int countBuildings(bool pending = true);
		int countBuildings(BWAPI::UnitType type, bool pending = true);
		
		int pendingBuildingsCount();
		int pendingBuildingsCount(BWAPI::UnitType type);
		
		void test()
		{
			std::cout << "ProductionManager!\n";
		}
	};
}
