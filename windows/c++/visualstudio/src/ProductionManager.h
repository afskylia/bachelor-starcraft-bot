#pragma once
#include <iostream>

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
		void test()
		{
			std::cout << "ProductionManager!\n";
		}
	};
}
