#include "Precomp.h"
#include "Core/GameEvaluateStep.h"

void RTS::GameEvaluateStep::GenerateCommandsFromEvaluations(const GameState& state,
	GameSimulationStep& nextStep) const
{
	ForEachCommandBuffer(
		[&]<typename T>(const CommandBuffer<T>&commandBuffer)
		{
			T::Execute(state, nextStep, commandBuffer.GetStoredCommands());
		});
}