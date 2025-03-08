#pragma once

namespace RTS::Constants
{
	static constexpr uint32 sNumSimulationStepsBetweenEvaluate = 10;
	static constexpr float sSimulationStepSize = .1f;
	static constexpr float sSimulationDurationSeconds = 60.0f;

	static constexpr float sEvaluateStepSize = sNumSimulationStepsBetweenEvaluate * sSimulationStepSize;
	static constexpr uint32 sTotalNumEvaluateSteps = static_cast<uint32>(sSimulationDurationSeconds / sEvaluateStepSize);
	static constexpr uint32 sTotalNumSimulationSteps = sTotalNumEvaluateSteps * sNumSimulationStepsBetweenEvaluate + 1;
}