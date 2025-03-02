#pragma once
#include "Utilities/Events.h"

namespace RTS
{
	struct OnUnitEvaluate :
		CE::EventType<OnUnitEvaluate>
	{
		OnUnitEvaluate() :
			EventType("OnUnitEvaluate")
		{
		}
	};
	inline OnUnitEvaluate sOnUnitEvaluate{};

	struct RTS
	{
		static void MoveTo(CE::World& world, entt::entity unit, entt::entity target);

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RTS);
	};
}
