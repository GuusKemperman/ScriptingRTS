#pragma once
#include "Core/UnitTypes.h"

namespace RTS
{
	template<UnitType::Enum>
	struct UnitTypeTag {};

	using TankTag = UnitTypeTag<UnitType::Tank>;
}