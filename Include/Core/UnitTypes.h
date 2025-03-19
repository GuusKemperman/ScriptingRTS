#pragma once
#include "WeaponTypes.h"

namespace RTS
{
	class UnitType
	{
	public:
		enum Enum : uint8
		{
			Tank,
			UNIT_TYPE_COUNT
		};

		std::string_view mDisplayName{};
		float mRadius{};
		float mMovementSpeed{};
		float mHealth{};
		float mHealthRegeneration{};
		WeaponType::Enum mWeaponType{};
	};

	static constexpr std::array<UnitType, UnitType::UNIT_TYPE_COUNT> sUnitTypes{
		UnitType{
			.mDisplayName = "Tank",
			.mRadius = .3f,
			.mMovementSpeed = 1.2f,
			.mHealth = 5000.0f,
			.mHealthRegeneration = 166.666667f,
			.mWeaponType = WeaponType::Rifle
		}
	};

	constexpr UnitType GetUnitType(UnitType::Enum type)
	{
		return sUnitTypes[type];
	}
}
