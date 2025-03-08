#include "Precomp.h"

#include "Components/SimulationComponent.h"
#include "Core/UnitTests.h"

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

		sim.StartSimulation([&steps](const RTS::GameSimulationStep& step) { steps.emplace_back(step); });
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
		for (size_t j = 1; j < simulations.size(); j++)
		{
			TEST_ASSERT(simulations[j].mSteps[i] == simulations[0].mSteps[i]);
		}
	}

	return CE::UnitTest::Success;
}