#pragma once
#include <Meta\Fwd\MetaReflectFwd.h>

namespace CE
{
	class World;
}

namespace RTS::Examples
{
	class BasicCombatComponent
	{
	public:
		static void OnUnitEvaluate(CE::World&, entt::entity);
	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(BasicCombatComponent);
	};
}
