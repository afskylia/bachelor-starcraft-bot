#pragma once

namespace MiraBot
{
	class StrategyManager
	{
		friend class Global;

	public:
		StrategyManager();
		void onFrame();
	};
}
