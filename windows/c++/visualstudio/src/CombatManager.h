#pragma once
#include <iostream>

namespace MiraBot
{
	class CombatManager
	{
		friend class Global;
	public:

		CombatManager();
		void onFrame();
		void rushEnemyBase();
		void test()
		{
			std::cout << "CombatManager!\n";
		}
	};
}
