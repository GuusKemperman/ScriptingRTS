#pragma once

namespace RTS
{
	class GameState;

	class MoveToCommand
	{
	public:
		static void Execute(GameState& state, std::span<const MoveToCommand> commands);

		entt::entity mUnit = entt::null;
		glm::vec2 mPosition{};
	};
}
