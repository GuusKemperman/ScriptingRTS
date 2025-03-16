#pragma once
#include "Components/TeamTag.h"
#include "Utilities/ComponentFilter.h"

namespace CE
{
	class World;
}

namespace RTS
{
	class GameSimulationStep;

	class GameState
	{
	public:
		GameState(CE::ComponentFilter team1Script, CE::ComponentFilter team2Script);

		GameState(const GameState&) = delete;
		GameState(GameState&&) noexcept;

		GameState& operator=(const GameState&) = delete;
		GameState& operator=(GameState&&) noexcept;

		~GameState();

		void Step(const GameSimulationStep& step);

		CE::World& GetWorld() { return *mWorld; }
		const CE::World& GetWorld() const { return *mWorld; }

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
		std::unique_ptr<CE::World> mWorld;
		uint32 mNumStepsCompleted{};

		CE::ComponentFilter mTeam1Script{};
		CE::ComponentFilter mTeam2Script{};
	};
}

