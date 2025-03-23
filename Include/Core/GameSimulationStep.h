#pragma once
#include "GameStepBase.h"
#include "Commands/CaptureObjectiveCommand.h"
#include "Commands/MoveToCommand.h"
#include "Commands/ShootAtCommand.h"
#include "Commands/SpawnObjectiveCommand.h"
#include "Commands/SpawnUnitCommand.h"

namespace RTS
{
	// Could also be a "using GameStep = ...", but now we can forward declare GameStep
	class GameSimulationStep final :
		public GameStepBase<SpawnUnitCommand, MoveToCommand, ShootAtCommand, CaptureObjectiveCommand, SpawnObjectiveCommand>
	{
	};
}