#pragma once
#include "GameStepBase.h"
#include "States/MoveToEntityState.h"
#include "States/ShootAtEntityState.h"

namespace RTS
{
	// Could also be a "using GameStep = ...", but now we can forward declare GameStep
	class GameEvaluateStep final :
		public GameStepBase<MoveToEntityState, ShootAtEntityState>
	{
	public:
		void GenerateCommandsFromEvaluations(const GameState& state, GameSimulationStep& nextStep) const;
	};
}
