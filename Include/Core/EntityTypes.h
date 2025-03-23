#pragma once

namespace RTS
{
	struct EntityType
	{
		enum Enum : bool
		{
			Unit,
			Objective
		};
	};

	struct UnitTag{};
	struct ObjectiveTag{};
}
