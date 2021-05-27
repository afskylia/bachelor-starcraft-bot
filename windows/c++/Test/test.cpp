#include "pch.h"

#include "../bwapi/include/BWAPI/UnitType.h"
#include "../visualstudio/src/Global.h"


TEST(TestCaseName, TestName)
{
	BWAPI::UnitType unit = BWAPI::UnitTypes::Protoss_Probe;
	MiraBot::Global::information().onUnitShow(unit.)
	EXPECT_EQ(1, 1);
	EXPECT_TRUE(true);
}
