#pragma once


#include <BWAPI.h>

class ProductionManager
{
public:

	ProductionManager();
	void onFrame();
	void buildGateway();
	void buildAttackUnits();
	void buildAdditionalSupply();
};
