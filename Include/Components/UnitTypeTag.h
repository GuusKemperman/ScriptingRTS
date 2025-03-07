#pragma once
#include "Core/UnitTypes.h"

namespace RTS
{
	template<UnitType::Enum>
	struct UnitTag {};

	using TankTag = UnitTag<UnitType::Tank>;
}