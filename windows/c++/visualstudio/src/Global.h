#pragma once
#include "InformationManager.h"

namespace MiraBot
{
	class MapTools;
	class WorkerManager;
	class ProductionManager;
	class CombatManager;

	class Global
	{
		MapTools* m_map_tools_ = nullptr;
		WorkerManager* m_worker_manager_ = nullptr;
		ProductionManager* m_production_manager_ = nullptr;
		CombatManager* m_combat_manager_ = nullptr;
		InformationManager* m_information_manager_ = nullptr;

		template <class T>
		static void reset(T*& ptr)
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
		static Global& instance();
		void init();


	public:
		static void gameStart();
		static WorkerManager& workers();
		static ProductionManager& production();
		static MapTools& map();
		static CombatManager& combat();
		static InformationManager& information();
	};
}
