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
	};
}
