#pragma once
#include "Components/TeamTag.h"
#include "Utilities/ComponentFilter.h"
#include "World/World.h"

namespace RTS
{
	class GameSimulationStep;

	class GameState
	{
	public:
		GameState(CE::ComponentFilter team1Script, CE::ComponentFilter team2Script);

		void Step(const GameSimulationStep& step);

		CE::World& GetWorld() { return mWorld; }
		const CE::World& GetWorld() const { return mWorld; }

		uint32 GetNumStepsCompleted() const { return mNumStepsCompleted; }

		const CE::ComponentFilter& GetTeamScript(TeamId team);

		enum class GameResult
		{
			Undetermined,
			Victory,
			Draw,
			Defeat
		};
		GameResult GetGameResult(TeamId team) const;

		bool IsGameOver() const;

		float GetScore(TeamId team) const;

	private:
		CE::World mWorld{ false };
		uint32 mNumStepsCompleted{};

		CE::ComponentFilter mTeam1Script{};
		CE::ComponentFilter mTeam2Script{};
	};
}

