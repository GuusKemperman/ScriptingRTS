#include "Precomp.h"
#include "Core/GameState.h"

#include "Assets/Level.h"
#include "Commands/CommandBuffer.h"
#include "Components/IsDestroyedTag.h"
#include "Components/ScriptExamples/BasicCombatComponent.h"
#include "Core/AssetManager.h"
#include "Core/GameSimulationStep.h"
#include "World/Registry.h"

RTS::GameState::GameState(CE::ComponentFilter team1Script, CE::ComponentFilter team2Script) :
	mWorld(std::make_unique<CE::World>(false)),
	mTeam1Script(team1Script == nullptr ? &CE::MetaManager::Get().GetType<Examples::BasicCombatComponent>() : team1Script),
	mTeam2Script(team2Script == nullptr ? &CE::MetaManager::Get().GetType<Examples::BasicCombatComponent>() : team2Script)
{
	CE::AssetHandle level = CE::AssetManager::Get().TryGetAsset<CE::Level>("L_SimulationStart");

	if (level == nullptr)
	{
		LOG(LogGame, Error, "No starting level for gamestate");
		return;
	}

	level->LoadIntoWorld(*mWorld);
}

RTS::GameState::GameState(GameState&&) noexcept = default;

RTS::GameState& RTS::GameState::operator=(GameState&&) noexcept = default;

RTS::GameState::~GameState() = default;

void RTS::GameState::Step(const GameSimulationStep& step)
{
	if (!GetWorld().HasBegunPlay())
	{
		GetWorld().BeginPlay();
	}

	step.ForEachCommandBuffer(
		[&]<typename T>(const CommandBuffer<T>& commandBuffer)
		{
			T::Execute(*this, commandBuffer.GetStoredCommands());
		});

	CE::Registry& reg = GetWorld().GetRegistry();

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
		if (glm::abs(scoreAllies - scoreEnemies) < 1.0f)
		{
			return GameResult::Draw;
		}
		if (scoreAllies < scoreEnemies)
		{
			return GameResult::Defeat;
		}
		return GameResult::Victory;
	}

	if (scoreAllies >= sWinningScore && scoreEnemies >= sWinningScore)
	{
		return GameResult::Draw;
	}
	if (scoreAllies >= sWinningScore)
	{
		return GameResult::Victory;
	}
	if (scoreEnemies >= sWinningScore)
	{
		return GameResult::Defeat;
	}

	return GameResult::Undetermined;
}

bool RTS::GameState::IsGameOver() const
{
	return GetGameResult(TeamId::Team1) != GameResult::Undetermined;
}

float RTS::GameState::GetScore(TeamId team) const
{
	switch (team)
	{
	case TeamId::Team1: return mTeam1Score;
	case TeamId::Team2: return mTeam2Score;
	default:
		return 0.0f;
	}
}

void RTS::GameState::SetScore(TeamId team, float score)
{
	switch (team)
	{
	case TeamId::Team1: mTeam1Score = score; break;
	case TeamId::Team2: mTeam2Score = score; break;
	default:
		break;
	}

}
