#include "Precomp.h"
#include "Core/GameState.h"

#include "Assets/Level.h"
#include "Commands/CommandBuffer.h"
#include "Core/AssetManager.h"
#include "Core/GameSimulationStep.h"
#include "World/Registry.h"

RTS::GameState::GameState()
{
	CE::AssetHandle level = CE::AssetManager::Get().TryGetAsset<CE::Level>("L_SimulationStart");

	if (level == nullptr)
	{
		LOG(LogGame, Error, "No starting level for gamestate");
		return;
	}

	level->LoadIntoWorld(mWorld);
}

void RTS::GameState::Step(const GameSimulationStep& step)
{
	if (!mWorld.HasBegunPlay())
	{
		mWorld.BeginPlay();
	}

	step.ForEachCommandBuffer(
		[&]<typename T>(const CommandBuffer<T>& commandBuffer)
		{
			T::Execute(*this, commandBuffer.GetStoredCommands());
		});

	mWorld.GetRegistry().RemovedDestroyed();
	mNumStepsCompleted++;
}
