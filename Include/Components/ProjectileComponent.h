#pragma once
#include "Core/WeaponTypes.h"

namespace RTS
{
	struct ProjectileComponent
	{
		bool operator==(const ProjectileComponent&) const = default;
		bool operator!=(const ProjectileComponent&) const = default;

		WeaponType::Enum mFiredFromWeapon{};
		entt::entity mTargetEntity = entt::null;

		glm::vec2 mSourcePosition{};
		glm::vec2 mTargetPosition{};

		int32 mNumStepsUntilImpact{};
		int8 mHasHit : 1{};
	};
}
