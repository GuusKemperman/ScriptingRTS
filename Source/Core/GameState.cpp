#include "Precomp.h"
#include "Core/GameState.h"

#include "Assets/Level.h"
#include "Commands/CommandBuffer.h"
#include "Components/IsDestroyedTag.h"
#include "Core/AssetManager.h"
#include "Core/GameSimulationStep.h"
#include "World/Registry.h"

RTS::GameState::GameState(CE::ComponentFilter team1Script, CE::ComponentFilter team2Script) :
	mTeam1Script(team1Script),
	mTeam2Script(team2Script)
{
	auto setScript = [](auto& script)
		{
			if (script == nullptr)
			{
				const CE::MetaType* playerScript = CE::MetaManager::Get().TryGetType("S_Unit");

				if (playerScript != nullptr)
				{
					script = playerScript;
				}
				else
				{
					LOG(LogGame, Error, "No player script!");
				}
			}
		};
	setScript(mTeam1Script);
	setScript(mTeam2Script);

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

const CE::ComponentFilter& RTS::GameState::GetTeamScript(TeamId team)
{
	if (team == TeamId::Team1)
	{
		return mTeam1Script;
	}
	return mTeam2Script;
}

RTS::GameState::GameResult RTS::GameState::GetGameResult(TeamId team) const
{
	if (mNumStepsCompleted == 0)
	{
		return GameResult::Undetermined;
	}

	const float scoreAllies = GetScore(team);
	const float scoreEnemies = GetScore(SwitchTeams(team));

	if (mNumStepsCompleted == Constants::sTotalNumSimulationSteps)
	{
		if (scoreAllies == scoreEnemies)
		{
			return GameResult::Draw;
		}
		if (scoreAllies < scoreEnemies)
		{
			return GameResult::Defeat;
		}
		return GameResult::Victory;
	}

	if (scoreAllies == 0.0f && scoreEnemies == 0.0f)
	{
		return GameResult::Draw;
	}
	if (scoreAllies == 0.0f)
	{
		return GameResult::Defeat;
	}
	if (scoreEnemies == 0.0f)
	{
		return GameResult::Victory;
	}

	return GameResult::Undetermined;
}

bool RTS::GameState::IsGameOver() const
{
	return GetGameResult(TeamId::Team1) != GameResult::Undetermined;
}

float RTS::GameState::GetScore(TeamId team) const
{
	const entt::sparse_set* storage = mWorld.GetRegistry().Storage(GetTeamTagStorage(team));
	return storage == nullptr ? 0.0f : static_cast<float>(storage->size());
}
