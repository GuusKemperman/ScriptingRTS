#pragma once
#include "Core/Constants.h"
#include "Utilities/Math.h"

#include "magic_enum/magic_enum.hpp"

namespace RTS
{
	enum class WeaponRange
	{
		Short,
		Mid,
		Long,
	};

	struct WeaponType
	{
		enum Enum : uint8
		{
			Rifle,
			WEAPON_TYPE_COUNT
		};

		union
		{
			struct NamedRange
			{
				float mShortRangeEnd{};
				float mMidRangeEnd{};
				float mLongRangeEnd{};
			} mNamedRanges;
			std::array<float, magic_enum::enum_count<WeaponRange>()> sRangeEnds{};
		} mRange;

		float mDamage{};
		float mFireCooldown{};
		int mNumStepsBetweenFiring = static_cast<int>(CE::Math::Round(mFireCooldown / Constants::sSimulationStepSize));
		float mProjectileTimeUntilImpact{};
		int mNumStepsUntilImpact = static_cast<int>(CE::Math::Round(mProjectileTimeUntilImpact / Constants::sSimulationStepSize));
	};

	static constexpr std::array<WeaponType, WeaponType::WEAPON_TYPE_COUNT> sWeaponTypes{
		WeaponType{
			.mRange = { 5.0f, 25.0f, 50.0f },
			.mDamage = 0.5f,
			.mFireCooldown = .5f,
			.mProjectileTimeUntilImpact = .4f
		}
	};

	constexpr auto GetWeaponType(WeaponType::Enum type)
	{
		return sWeaponTypes[type];
	}
}
