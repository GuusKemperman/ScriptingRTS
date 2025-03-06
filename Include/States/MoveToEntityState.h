#pragma once

namespace RTS
{
	class GameState;
	class GameSimulationStep;
	class GameEvaluateStep;

	class MoveToEntityState
	{
	public:
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const MoveToEntityState> commands);

		static void EnterState(const GameState& gameState, 
			GameEvaluateStep& nextStep,
			entt::entity unit, 
			entt::entity targetEntity);

		bool operator==(const MoveToEntityState&) const = default;
		bool operator!=(const MoveToEntityState&) const = default;

		entt::entity mUnit = entt::null;
		entt::entity mTargetEntity = entt::null;
	};
}
