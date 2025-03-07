#pragma once
#include "Components/TeamTag.h"
#include "Core/UnitTypes.h"

namespace RTS
{
	class GameState;

	class SpawnUnitCommand
	{
	public:
		static void Execute(GameState& state, std::span<const SpawnUnitCommand> commands);

		bool operator==(const SpawnUnitCommand&) const = default;
		bool operator!=(const SpawnUnitCommand&) const = default;

		glm::vec2 mPosition{};
		TeamId mTeamId{};
		UnitType::Enum mUnitType{};
	};
}
