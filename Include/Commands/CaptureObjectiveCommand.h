#pragma once

namespace RTS
{
	class GameState;

	class CaptureObjectiveCommand
	{
	public:
		static void Execute(GameState& state, std::span<const CaptureObjectiveCommand> commands);

		bool operator==(const CaptureObjectiveCommand&) const = default;
		bool operator!=(const CaptureObjectiveCommand&) const = default;

		entt::entity mObjective = entt::null;
		uint32 mNumFromTeam1{};
		uint32 mNumFromTeam2{};
	};
}
