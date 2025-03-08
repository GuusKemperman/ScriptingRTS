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
		static void MoveToEntity(entt::entity target);

		static void MoveToPosition(glm::vec2 target);

		static entt::entity FindEntity(UnitFilter filter);

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RTSAPI);
	};
}
