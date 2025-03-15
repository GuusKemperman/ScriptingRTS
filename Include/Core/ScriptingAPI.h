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

	enum class Action
	{
		MoveTo,
		FleeFrom,
		ShootAt
	};

	struct RTSAPI
	{
		static bool Action(Action action, UnitFilter target);

		static bool Condition(UnitFilter target);

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RTSAPI);
	};
}

template<>
struct Reflector<RTS::Action>
{
	static CE::MetaType Reflect();
};
REFLECT_AT_START_UP(rtsAction, RTS::Action);
