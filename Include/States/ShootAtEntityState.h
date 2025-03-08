#pragma once

namespace RTS
{
	class GameState;
	class GameSimulationStep;
	class GameEvaluateStep;

	class ShootAtEntityState
	{
	public:
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const ShootAtEntityState> commands);

		static void EnterState(const GameState& gameState,
			GameEvaluateStep& nextStep,
			entt::entity unit,
			entt::entity targetEntity);

		bool operator==(const ShootAtEntityState&) const = default;
		bool operator!=(const ShootAtEntityState&) const = default;

		entt::entity mUnit = entt::null;
		entt::entity mTargetEntity = entt::null;
	};
}
