#pragma once
#include "Core/Constants.h"
#include "Utilities/Math.h"

namespace RTS
{
	struct WeaponType
	{
		enum Enum : uint8
		{
			Rifle,
			WEAPON_TYPE_COUNT
		};

		float mMaxRange{};
		float mDamage{};
		float mFireCooldown{};
		int mNumStepsBetweenFiring = static_cast<int>(CE::Math::Round(mFireCooldown / Constants::sSimulationStepSize));
		float mProjectileTimeUntilImpact{};
		int mNumStepsUntilImpact = static_cast<int>(CE::Math::Round(mProjectileTimeUntilImpact / Constants::sSimulationStepSize));
	};

	static constexpr std::array<WeaponType, WeaponType::WEAPON_TYPE_COUNT> sWeaponTypes{
		WeaponType{ .mMaxRange = 50.0f, .mDamage = 1.0f, .mFireCooldown = 1.0f, .mProjectileTimeUntilImpact = 1.0f }
	};

	template<auto PtrToMember>
	constexpr auto GetWeaponProperty(WeaponType::Enum type)
	{
		return sWeaponTypes[type].*PtrToMember;
	}
}
