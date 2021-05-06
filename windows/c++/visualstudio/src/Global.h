#pragma once

namespace MiraBot
{
	class MapTools;
	class WorkerManager;
	class ProductionManager;
	class CombatManager;

	class Global
	{
		MapTools* m_mapTools = nullptr;
		WorkerManager* m_workerManager = nullptr;
		ProductionManager* m_productionManager = nullptr;
		CombatManager* m_combatManager = nullptr;

		template <class T>
		void reset(T*& ptr)
		{
			delete ptr;
			ptr = nullptr;
		}

		template <class T>
		static T* get(T*& ptr)
		{
			if (ptr == nullptr) { ptr = new T(); }
			return ptr;
		}

		Global();
		static Global& Instance();
		void init();


	public:
		static void GameStart();
		static WorkerManager& Workers();
		static ProductionManager& Production();
		static MapTools& Map();
		static CombatManager& Combat();
	};
}
