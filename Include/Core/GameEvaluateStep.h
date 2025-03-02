#pragma once
#include "GameStepBase.h"
#include "States/MoveToEntityState.h"

namespace RTS
{
	// Could also be a "using GameStep = ...", but now we can forward declare GameStep
	class GameEvaluateStep final :
		public GameStepBase<MoveToEntityState>
	{
	public:
		void GenerateCommandsFromEvaluations(const GameState& state, GameSimulationStep& nextStep) const;
	};
}
