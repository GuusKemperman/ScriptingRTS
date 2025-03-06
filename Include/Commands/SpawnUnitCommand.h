#pragma once
#include "Components/TeamTag.h"

namespace RTS
{
	class GameState;

	class SpawnUnitCommand
	{
	public:
		static void Execute(GameState& state, std::span<const SpawnUnitCommand> commands);

		glm::vec2 mPosition{};
		TeamId mTeamId{};
	};
}
