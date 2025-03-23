#pragma once

namespace RTS
{
	class GameState;

	class SpawnObjectiveCommand
	{
	public:
		static void Execute(GameState& state, std::span<const SpawnObjectiveCommand> commands);

		bool operator==(const SpawnObjectiveCommand&) const = default;
		bool operator!=(const SpawnObjectiveCommand&) const = default;

		glm::vec2 mLocation{};
	};
}
