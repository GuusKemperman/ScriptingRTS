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
		WeaponType::Enum mWeaponType{};
	};

	static constexpr std::array<UnitType, UnitType::UNIT_TYPE_COUNT> sUnitTypes{
		UnitType{ .mDisplayName = "Tank", .mRadius = 1.0f, .mMovementSpeed = 4.0f, .mHealth = 5.0f, .mWeaponType = WeaponType::Rifle }
	};

	constexpr UnitType GetUnitType(UnitType::Enum type)
	{
		return sUnitTypes[type];
	}

	template<auto PtrToMember>
	constexpr auto GetUnitProperty(UnitType::Enum type)
	{
		return sUnitTypes[type].*PtrToMember;
	}

}
