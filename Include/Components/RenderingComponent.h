#pragma once
#include "TeamTag.h"
#include "BasicDataTypes/Colors/LinearColor.h"
#include "Meta/Fwd/MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace RTS
{
	class RenderingComponent
	{
	public:
		void OnSimulationStart(CE::World& viewportWorld, entt::entity owner);

		float mTimeStamp{};
		float mPlaySpeed = 1.0f;

		CE::LinearColor mFriendlyColour{ 0.0f, 0.0f, 1.0f, 1.0f };
		CE::LinearColor mEnemyColour{ 1.0f, 0.0f, 0.0f, 1.0f };

		TeamId mDisplayForTeam = TeamId::Team1;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RenderingComponent);
	};
}
