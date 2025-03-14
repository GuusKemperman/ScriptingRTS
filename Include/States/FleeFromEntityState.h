#pragma once
#include "TargetEntityWhileConditionState.h"

namespace RTS
{
	class FleeFromEntityState :
		public TargetEntityWhileConditionState<FleeFromEntityState>
	{
	public:
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const FleeFromEntityState> commands);

		bool operator==(const FleeFromEntityState&) const = default;
		bool operator!=(const FleeFromEntityState&) const = default;
	};
}
