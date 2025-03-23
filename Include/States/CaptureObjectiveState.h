#pragma once

namespace RTS
{
	class GameState;
	class GameSimulationStep;

	struct CaptureObjectiveState
	{
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const CaptureObjectiveState> commands);
	};
}