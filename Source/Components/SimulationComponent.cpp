#include "Precomp.h"
#include "Components/SimulationComponent.h"

#include "Core/ScriptingAPI.h"
#include "Utilities/Reflect/ReflectComponentType.h"
#include "World/EventManager.h"

namespace
{
	struct InternalSimulationSystem : CE::System
	{
		InternalSimulationSystem(RTS::SimulationComponent& component) : mOuterSimulationComponent(component) {}

		RTS::SimulationComponent& mOuterSimulationComponent;
	};
}

void RTS::SimulationComponent::OnBeginPlay([[maybe_unused]] CE::World& viewportWorld, entt::entity)
{
	mThread = std::jthread{ [this](const std::stop_token& token) { SimulateThread(token); } };
}

const RTS::SimulationComponent* RTS::SimulationComponent::TryGetOwningSimulationComponent(const CE::World& ownedWorld)
{
	const InternalSimulationSystem* system = ownedWorld.TryGetSystem<InternalSimulationSystem>();

	if (system != nullptr)
	{
		return &system->mOuterSimulationComponent;
	}
	return nullptr;
}

RTS::SimulationComponent* RTS::SimulationComponent::TryGetOwningSimulationComponent(CE::World& ownedWorld)
{
	return const_cast<SimulationComponent*>(TryGetOwningSimulationComponent(const_cast<const CE::World&>(ownedWorld)));
}

void RTS::SimulationComponent::SimulateThread(const std::stop_token& stop)
{
	mCurrentState.GetWorld().AddSystem<InternalSimulationSystem>(*this);

	auto clearBuffers = []<typename T>(CommandBuffer<T>&buffer)
	{
		buffer.Clear();
	};

	auto reserveBuffers = []<typename T>(CommandBuffer<T>&buffer, size_t size)
	{
		buffer.Reserve(size);
	};

	mEvaluateStep.ForEachCommandBuffer(reserveBuffers, static_cast<size_t>(mStartingTotalNumOfUnits));
	mSimulateStep.ForEachCommandBuffer(reserveBuffers, static_cast<size_t>(mStartingTotalNumOfUnits));

	{ // TODO replace hardcoding of spawning of units
		for (uint32 i = 0; i < mStartingTotalNumOfUnits; i++)
		{
			glm::vec2 pos = glm::vec2{ 25.0f } *static_cast<float>(i);
			mSimulateStep.AddCommand(SpawnUnitCommand{ pos });
		}
		mCurrentState.Step(mSimulateStep);

		if (mOnStepCompletedCallback)
		{
			mOnStepCompletedCallback(mSimulateStep);
		}
	}

	for (int evaluateStepNum = 0; evaluateStepNum < sTotalNumEvaluateSteps; evaluateStepNum++)
	{
		CE::World& world = mCurrentState.GetWorld();

		mEvaluateStep.ForEachCommandBuffer(clearBuffers);
		world.GetEventManager().InvokeEventsForAllComponents(sOnUnitEvaluate);

		for (int simulateStepNum = 0; simulateStepNum < sNumSimulationStepsBetweenEvaluate; simulateStepNum++)
		{
			if (stop.stop_requested())
			{
				goto exit;
			}

			mSimulateStep.ForEachCommandBuffer(clearBuffers);

			// TODO Physics

			mEvaluateStep.GenerateCommandsFromEvaluations(mCurrentState, mSimulateStep);
			mCurrentState.Step(mSimulateStep);

			if (mOnStepCompletedCallback)
			{
				mOnStepCompletedCallback(mSimulateStep);
			}
		}
	}
exit:;
}

CE::MetaType RTS::SimulationComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<SimulationComponent>{}, "SimulationComponent" };

	metaType.AddField(&SimulationComponent::mStartingTotalNumOfUnits, "mStartingTotalNumOfUnits");

	CE::BindEvent(metaType, CE::sOnBeginPlay, &SimulationComponent::OnBeginPlay);

	CE::ReflectComponentType<SimulationComponent>(metaType);

	return metaType;
}
