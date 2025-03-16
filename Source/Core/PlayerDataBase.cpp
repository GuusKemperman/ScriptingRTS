#include "Precomp.h"
#include "Core/PlayerDataBase.h"

#include "Components/SimulationComponent.h"
#include "Utilities/Random.h"

void RTS::PlayerDataBase::AddPlayer(CompiledProgram program, std::string name)
{
	std::shared_ptr<Player> player = std::make_shared<Player>(std::move(program), std::move(name));

	size_t numBattles = std::min(GetNumberOfPlayers(), 50ull);
	for (int i = 0; i < numBattles; i++)
	{
		std::shared_ptr opponent = FindMatch(*player);

		if (opponent == nullptr)
		{
			continue;
		}

		GameState battleResult = Battle(*player, *opponent);
		UpdateELO(*player, *opponent, battleResult);
	}
	
	std::unique_lock l{ mPlayersMutex };
	mPlayers.emplace_back(std::move(player));
}

size_t RTS::PlayerDataBase::GetNumberOfPlayers() const
{
	std::shared_lock l{ mPlayersMutex };
	return mPlayers.size();
}

std::pair<std::shared_lock<std::shared_mutex>, const std::vector<std::shared_ptr<RTS::PlayerDataBase::Player>>&> RTS::PlayerDataBase::OpenForRead() const
{
	return { std::shared_lock{ mPlayersMutex }, mPlayers };
}

std::pair<std::unique_lock<std::shared_mutex>, std::vector<std::shared_ptr<RTS::PlayerDataBase::Player>>&> RTS::PlayerDataBase::OpenForWrite()
{
	return { std::unique_lock{ mPlayersMutex }, mPlayers };
}

std::shared_ptr<RTS::PlayerDataBase::Player> RTS::PlayerDataBase::FindMatch([[maybe_unused]] const Player& forPlayer)
{
	std::shared_lock l{ mPlayersMutex };

	if (mPlayers.empty())
	{
		return nullptr;
	}
	return mPlayers[CE::Random::Range<size_t>(0, mPlayers.size())];
}

RTS::GameState RTS::PlayerDataBase::Battle(const Player& team1, const Player& team2)
{
	SimulationComponent sim{};
	sim.mUseMultiThreading = false;
	sim.mRunOnCallingThread = true;
	sim.mUsePhysics = false;
	sim.mStartingTotalNumOfUnits = 2;

	sim.mInvokeEvaluateEvents =
		[&]()
		{
			const CE::Registry& reg = sim.GetGameState().GetWorld().GetRegistry();

			for (entt::entity entity : reg.View<Team1Tag>())
			{
				sim.OnPreEvaluate(entity);
				team1.mProgram.Run();
			}

			for (entt::entity entity : reg.View<Team2Tag>())
			{
				sim.OnPreEvaluate(entity);
				team2.mProgram.Run();
			}
		};

	sim.StartSimulation();
	return std::move(sim.GetGameState());
}

void RTS::PlayerDataBase::UpdateELO(Player& team1, Player& team2, const GameState& result)
{
	std::shared_lock l1{ team1.mEloMutex };
	std::shared_lock l2{ team2.mEloMutex };

	float K1 = 800.0f / static_cast<float>(team1.mGamesPlayed + 1);
	float K2 = 800.0f / static_cast<float>(team2.mGamesPlayed + 1);

	float r1 = team1.mElo;
	float r2 = team2.mElo;

	// Calculate expected scores
	float exponent = (r2 - r1) / 400.0f;
	float e1 = 1.0f / (1.0f + std::pow(10.0f, exponent));
	float e2 = 1.0f - e1;

	float s1{};
	float s2{};
	switch (result.GetGameResult(TeamId::Team1))
	{
	case GameState::GameResult::Undetermined:
	case GameState::GameResult::Draw:
		s1 = 0.5f;
		s2 = 0.5f;
		break;
	case GameState::GameResult::Victory:
		s1 = 1.0f;
		s2 = 0.0f;
		break;
	case GameState::GameResult::Defeat:
		s1 = 0.0f;
		s2 = 1.0f;
		break;
	}

	// Calculate rating changes using individual K factors
	float delta1 = K1 * (s1 - e1);
	float delta2 = K2 * (s2 - e2);

	// Update ELO ratings
	team1.mElo += delta1;
	team2.mElo += delta2;

	// Increment games played count
	team1.mGamesPlayed++;
	team2.mGamesPlayed++;
}
