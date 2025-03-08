#include "Precomp.h"
#include "Core/GameState.h"

#include "Assets/Level.h"
#include "Commands/CommandBuffer.h"
#include "Components/IsDestroyedTag.h"
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

	CE::Registry& reg = mWorld.GetRegistry();

	// The order that entities are destroyed in affects
	// the iteration order and (i believe) the distribution
	// of future entity ids. Iteration order affects physics resolution.
	// To ensure determinism, we always destroy in the same order.
	reg.Storage<CE::IsDestroyedTag>().sort(
		[&](const entt::entity lhs, const entt::entity rhs)
		{
			return entt::to_integral(lhs) < entt::to_integral(rhs);
		});

	reg.RemovedDestroyed();
	mNumStepsCompleted++;
}
