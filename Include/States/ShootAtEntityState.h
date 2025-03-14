#pragma once
#include "TargetEntityWhileConditionState.h"

namespace RTS
{
	class ShootAtEntityState :
		public TargetEntityWhileConditionState<ShootAtEntityState>
	{
	public:
		static void Execute(const GameState& gameState,
			GameSimulationStep& nextStep,
			std::span<const ShootAtEntityState> commands);

		bool operator==(const ShootAtEntityState&) const = default;
		bool operator!=(const ShootAtEntityState&) const = default;
	};
}
