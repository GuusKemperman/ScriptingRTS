#include "Precomp.h"
#include "Core/PlayerDataBase.h"

#include "Components/SimulationComponent.h"
#include "Utilities/Random.h"

void RTS::PlayerDataBase::AddPlayer(CompiledProgram program, std::string name)
{
	std::shared_ptr<Player> player = std::make_shared<Player>(std::move(program), std::move(name));

	for (size_t i = 0; i < GetNumberOfPlayers(); i++)
	{
		std::shared_ptr opponent = 
			[&]() -> std::shared_ptr<Player>
			{
				std::shared_lock l{ mPlayersMutex };

				if (i < mPlayers.size())
				{
					return mPlayers[i];
				}
				return nullptr;
			}();

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

std::shared_ptr<RTS::PlayerDataBase::Player> RTS::PlayerDataBase::FindMatch(const Player& forPlayer)
{
	std::shared_lock l{ mPlayersMutex };

	if (mPlayers.size() <= 1)
	{
		return nullptr;
	}

	size_t randomIndex = CE::Random::Range<size_t>(0, mPlayers.size());
	const std::shared_ptr<Player>& opponent = mPlayers[randomIndex];

	if (opponent.get() == &forPlayer)
	{
		++randomIndex;
		return mPlayers[randomIndex % mPlayers.size()];
	}

	return opponent;
}

RTS::GameState RTS::PlayerDataBase::Battle(const Player& team1, const Player& team2)
{
	SimulationComponent sim{};
	sim.mUseMultiThreading = false;
	sim.mRunOnCallingThread = true;
	sim.mUsePhysics = true;
	sim.mStartingTotalNumOfUnits = 16;

	sim.SetCompiledScripts(team1.mProgram, team2.mProgram);

	sim.StartSimulation();
	return std::move(sim.GetGameState());
}

void RTS::PlayerDataBase::UpdateELO(Player& team1, Player& team2, const GameState& result)
{
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

	std::unique_lock l1{ team1.mEloMutex };
	std::unique_lock l2{ team2.mEloMutex };

	float K1 = 800.0f / static_cast<float>(team1.mGamesPlayed + 1);
	float K2 = 800.0f / static_cast<float>(team2.mGamesPlayed + 1);

	float r1 = team1.mElo;
	float r2 = team2.mElo;

	// Calculate expected scores
	float exponent = (r2 - r1) / 400.0f;
	float e1 = 1.0f / (1.0f + std::pow(10.0f, exponent));
	float e2 = 1.0f - e1;

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
