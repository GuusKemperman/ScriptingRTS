#pragma once
#include "UnitFilter.h"
#include "Utilities/Events.h"

namespace RTS
{
	class GameEvaluateStep;
}

namespace RTS
{
	class GameState;
	class SimulationComponent;

	struct OnUnitEvaluate :
		CE::EventType<OnUnitEvaluate>
	{
		OnUnitEvaluate() :
			EventType("OnUnitEvaluate")
		{
		}
	};
	inline OnUnitEvaluate sOnUnitEvaluate{};

	namespace Internal
	{
		struct OnUnitEvaluateTarget
		{
			const GameState* sCurrentState{};
			GameEvaluateStep* sNextStep{};
			entt::entity sCurrentUnit{};
		};
		void SetOnUnitEvaluateTargetForCurrentThread(OnUnitEvaluateTarget target);
	}

	struct RTSAPI
	{
		static void MoveToEntity(UnitFilter target);

		static void ShootAt(UnitFilter target);

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RTSAPI);
	};
}
