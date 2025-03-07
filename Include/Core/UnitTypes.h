#pragma once

namespace RTS
{
	struct UnitType
	{
		enum Enum : uint8
		{
			Tank,
			UNIT_TYPE_COUNT
		};

		std::string_view mDisplayName{};
		float mRadius{};
		float mMovementSpeed{};
	};

	static constexpr std::array<UnitType, UnitType::UNIT_TYPE_COUNT> sUnitTypes{
		UnitType{ .mDisplayName = "Tank", .mRadius = 1.0f, .mMovementSpeed = 4.0f }
	};

	template<auto PtrToMember>
	constexpr auto GetUnitProperty(UnitType::Enum type)
	{
		return sUnitTypes[type].*PtrToMember;
	}

}