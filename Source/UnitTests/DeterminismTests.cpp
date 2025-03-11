#include "Precomp.h"

#include "Components/SimulationComponent.h"
#include "Core/UnitTests.h"
#include "World/Archiver.h"

using namespace CE;

UNIT_TEST(Determinism, EqualSteps)
{
	struct SimResult
	{
		RTS::SimulationComponent mSimulation{};
		std::vector<RTS::GameSimulationStep> mSteps{};
	};

	std::array<SimResult, 2> simulations{};

	for (auto& [sim, steps] : simulations)
	{
		sim.mStartingTotalNumOfUnits = 100;

		sim.StartSimulation(
			[&steps](const RTS::GameSimulationStep& step)
			{
				steps.emplace_back(step);
			});
	}

	for (auto& [sim, steps] : simulations)
	{
		sim.WaitForComplete();
	}

	for (auto& [sim, steps] : simulations)
	{
		TEST_ASSERT(steps.size() == RTS::Constants::sTotalNumSimulationSteps);
	}

	for (uint32 i = 0; i < RTS::Constants::sTotalNumSimulationSteps; i++)
	{
		RTS::GameSimulationStep& step1 = simulations[0].mSteps[i];
		RTS::GameSimulationStep& step2 = simulations[1].mSteps[i];

		step1.ForEachCommandBuffer(
			[&]<typename T>(RTS::CommandBuffer<T>& buffer1)
		{
			RTS::CommandBuffer<T>& buffer2 = step2.GetBuffer<T>();

			std::list<T> cpy1{ buffer1.GetStoredCommands().data(), buffer1.GetStoredCommands().data() + buffer1.GetNumSubmittedCommands() };
			std::list<T> cpy2{ buffer2.GetStoredCommands().data(), buffer2.GetStoredCommands().data() + buffer2.GetNumSubmittedCommands() };

			for (auto it1 = cpy1.begin(); it1 != cpy1.end();)
			{
				auto it2 = std::find(cpy2.begin(), cpy2.end(), *it1);

				TEST_NOT_EQUAL(it2, cpy2.end());

				cpy2.erase(it2);
				it1 = cpy1.erase(it1);
			}

			TEST_ASSERT(cpy2.empty());
		});
	}
}

UNIT_TEST(Determinism, ApplySteps)
{
	RTS::SimulationComponent sim{};
	sim.mStartingTotalNumOfUnits = 1000;
	RTS::GameState gameState{};

	sim.StartSimulation(
		[&](const RTS::GameSimulationStep& step)
		{
			gameState.Step(step);

			BinaryGSONObject realWorld = Archiver::Serialize(sim.GetGameState().GetWorld());
			BinaryGSONObject replicatedWorld = Archiver::Serialize(gameState.GetWorld());

			TEST_EQUAL(realWorld, replicatedWorld);
		});
	sim.WaitForComplete();
}