#pragma once
#include "Commands/CommandBuffer.h"

namespace RTS
{
	class MoveToCommand;
}

namespace RTS
{
	class GameState;
	class GameSimulationStep;
	class GameEvaluateStep;

	class MoveToPositionState
	{
	public:
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const MoveToPositionState> commands);

		static void EnterState(const GameState& gameState,
			GameEvaluateStep& nextStep,
			entt::entity unit,
			glm::vec2 targetPosition);




		entt::entity mUnit = entt::null;
		glm::vec2 mTargetPosition{};
	};
}
