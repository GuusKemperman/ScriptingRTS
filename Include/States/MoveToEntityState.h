#pragma once
#include "TargetEntityWhileConditionState.h"

namespace RTS
{
	class MoveToEntityState :
		public TargetEntityWhileConditionState<MoveToEntityState>
	{
	public:
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const MoveToEntityState> commands);

		bool operator==(const MoveToEntityState&) const = default;
		bool operator!=(const MoveToEntityState&) const = default;
	};
}
