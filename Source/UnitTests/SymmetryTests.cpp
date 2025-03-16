#include "Precomp.h"

#include "Components/SimulationComponent.h"
#include "Core/UnitTests.h"
#include "Utilities/Random.h"
#include "World/Registry.h"

namespace
{
	bool IsMirroredPos(glm::vec2 pos1, glm::vec2 pos2)
	{
		return pos1 == -pos2;
	}

	struct TestParams
	{
		bool mUsePhysics = true;
		bool mUseMultiThreading = true;
	};
	void TestSymmetry(TestParams params)
	{
		std::optional<std::vector<std::pair<entt::entity, entt::entity>>> pairs{};
		RTS::SimulationComponent sim{};

		sim.mUsePhysics = params.mUsePhysics;
		sim.mUseMultiThreading = params.mUseMultiThreading;
		sim.mRunOnCallingThread = true;
		sim.mOnStepCompletedCallback = [&](const RTS::GameSimulationStep&)
			{
				const CE::Registry& reg = sim.GetGameState().GetWorld().GetRegistry();

				if (!pairs.has_value())
				{
					pairs.emplace();

					for (auto [entity1, disk1] : reg.View<CE::TransformedDiskColliderComponent, RTS::Team1Tag>().each())
					{
						bool wasMirroredSpawned = false;
						for (auto [entity2, disk2] : reg.View<CE::TransformedDiskColliderComponent, RTS::Team2Tag>().each())
						{
							if (IsMirroredPos(disk1.mCentre, disk2.mCentre))
							{
								pairs->emplace_back(entity1, entity2);
								wasMirroredSpawned = true;
								break;
							}
						}
						TEST_ASSERT(wasMirroredSpawned);
					}
					TEST_EQUAL(pairs->size(), static_cast<size_t>(sim.mStartingTotalNumOfUnits / 2));
					return;
				}

				auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
				TEST_NOT_NULL(diskStorage);
				auto* rngStorage = reg.Storage<CE::DefaultRandomEngine>();
				TEST_NOT_NULL(rngStorage);

				for (auto [entity1, entity2] : *pairs)
				{
					TEST_EQUAL(diskStorage->contains(entity1), diskStorage->contains(entity2));
					TEST_EQUAL(rngStorage->contains(entity1), rngStorage->contains(entity2));

					if (!diskStorage->contains(entity1))
					{
						continue;
					}

					auto rng1 = rngStorage->get(entity1);
					auto rng2 = rngStorage->get(entity2);

					TEST_EQUAL(rng1(), rng2());

					auto& disk1 = diskStorage->get(entity1);
					auto& disk2 = diskStorage->get(entity2);

					TEST_ASSERT(IsMirroredPos(disk1.mCentre, disk2.mCentre));
				}
			};

		sim.StartSimulation();
	}
}

UNIT_TEST(Symmetry, DefaultSettings)
{
	TestSymmetry({});
}

UNIT_TEST(Symmetry, NoPhysics)
{
	TestSymmetry({ .mUsePhysics = false });
}

UNIT_TEST(Symmetry, NoMultiThreading)
{
	TestSymmetry({ .mUseMultiThreading = false });
}


UNIT_TEST(Symmetry, NoPhysicsNoMultiThreading)
{
	TestSymmetry({ .mUsePhysics = false, .mUseMultiThreading = false });
}
