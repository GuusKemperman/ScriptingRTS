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
		} mRangeDistance;

		union
		{
			struct NamedRange
			{
				float mShortRangeAccuracy{};
				float mMidRangeAccuracy{};
				float mLongRangeAccuracy{};
			} mNamedRanges;
			std::array<float, magic_enum::enum_count<WeaponRange>()> sAccuracies{};
		} mRangeAccuracy;

		float mDamage{};
		float mFireCooldown{};
		int mNumStepsBetweenFiring = static_cast<int>(CE::Math::Round(mFireCooldown / Constants::sSimulationStepSize));
		float mProjectileTimeUntilImpact{};
		int mNumStepsUntilImpact = static_cast<int>(CE::Math::Round(mProjectileTimeUntilImpact / Constants::sSimulationStepSize));
	};

	static constexpr std::array<WeaponType, WeaponType::WEAPON_TYPE_COUNT> sWeaponTypes{
		WeaponType{
			.mRangeDistance = { 3.0f, 8.0f, 15.0f },
			.mRangeAccuracy = { 0.95f, 0.55f, 0.15f },
			.mDamage = 1800.0f,
			.mFireCooldown = .3f,
			.mProjectileTimeUntilImpact = .4f
		}
	};

	constexpr auto GetWeaponType(WeaponType::Enum type)
	{
		return sWeaponTypes[type];
	}
}
