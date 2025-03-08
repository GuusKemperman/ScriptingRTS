#pragma once

namespace RTS
{
	class GameState;

	class ShootAtCommand
	{
	public:
		static void Execute(GameState& state, std::span<const ShootAtCommand> commands);

		bool operator==(const ShootAtCommand&) const = default;
		bool operator!=(const ShootAtCommand&) const = default;

		entt::entity mFiredBy = entt::null;
		entt::entity mTargetEntity = entt::null;
	};
}
