#pragma once
#include "World/World.h"

namespace RTS
{
	class GameSimulationStep;

	class GameState
	{
	public:
		GameState();

		void Step(const GameSimulationStep& step);

		CE::World& GetWorld() { return mWorld; }
		const CE::World& GetWorld() const { return mWorld; }

	private:
		CE::World mWorld{ false };
		uint32 mNumStepsCompleted{};
		uint32 mScore{};
	};
}

