#include "BuildOrderData.h"

using namespace MiraBot;

BuildOrderData::BuildOrderData() = default;

bool BuildOrderData::Cmp::operator()(const std::pair<double, BWAPI::UnitType>& a,
                                     const std::pair<double, BWAPI::UnitType>& b) const
{
	return a.first > b.first;
}
