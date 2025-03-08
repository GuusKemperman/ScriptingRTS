#include "Precomp.h"
#include "Components/SimulationComponent.h"

#include <entt/entity/runtime_view.hpp>

#include "Components/RenderingComponent.h"
#include "Core/ScriptingAPI.h"
#include "Systems/RenderingSystem.h"
#include "Utilities/Reflect/ReflectComponentType.h"
#include "World/EventManager.h"
#include "World/Physics.h"

void RTS::SimulationComponent::OnBeginPlay(CE::World& viewportWorld, entt::entity)
{
	std::function<void(const GameSimulationStep&)> callback{};
	if (!viewportWorld.GetRegistry().Storage<RenderingComponent>().empty())
	{
		callback =
			[renderer = &viewportWorld.AddSystem<RenderingSystem>()](const GameSimulationStep& step)
			{
				renderer->RecordStep(step);
			};
	}

	StartSimulation(std::move(callback));
}

void RTS::SimulationComponent::StartSimulation(std::function<void(const GameSimulationStep&)> onStepCompleted)
{
	mOnStepCompletedCallback = std::move(onStepCompleted);
	mThread = std::jthread{ [this](const std::stop_token& token) { SimulateThread(token); } };
}

void RTS::SimulationComponent::WaitForComplete()
{
	mThread.join();
}

void RTS::SimulationComponent::InvokeEvaluateEvents()
{
	CE::World& world = mCurrentState.GetWorld();
	for (const CE::BoundEvent& boundEvent : world.GetEventManager().GetBoundEvents(sOnUnitEvaluate))
	{
		entt::sparse_set* const storage = world.GetRegistry().Storage(boundEvent.mType.get().GetTypeId());

		if (storage == nullptr)
		{
			continue;
		}

		const CE::MetaFunc& func = boundEvent.mFunc.get();
		static constexpr std::array argForms = { CE::TypeForm::Ref, CE::TypeForm::Ref, CE::TypeForm::ConstRef };

		entt::runtime_view view{};
		view.iterate(*storage);

		std::for_each(std::execution::par_unseq, view.begin(), view.end(),
			[&](entt::entity entity)
			{
				std::array args{
					CE::MetaAny{ boundEvent.mType, storage->value(entity), false },
					CE::MetaAny{ world },
					CE::MetaAny{ entity }
				};

				Internal::OnUnitEvaluateTarget target{
					.sCurrentState = &mCurrentState,
					.sNextStep = &mEvaluateStep,
					.sCurrentUnit = entity
				};
				Internal::SetOnUnitEvaluateTargetForCurrentThread(target);

				func.InvokeUnchecked(args, argForms);
			});
	}
}

void RTS::SimulationComponent::SimulateThread(const std::stop_token& stop)
{
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

		auto spawnUnitsForTeam = [&](float offsetMul, TeamId team)
			{
				for (uint32 i = 0; i < mStartingTotalNumOfUnits / 2; i++)
				{
					glm::vec2 pos = offsetMul * glm::vec2{ 1.4f } * static_cast<float>(i + 10);
					mSimulateStep.AddCommand(SpawnUnitCommand{ pos, team, UnitType::Tank });
				}
			};

		spawnUnitsForTeam(1.0f, TeamId::Team1);
		spawnUnitsForTeam(-1.0f, TeamId::Team2);

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
		InvokeEvaluateEvents();

		for (int simulateStepNum = 0; simulateStepNum < sNumSimulationStepsBetweenEvaluate; simulateStepNum++)
		{
			if (stop.stop_requested())
			{
				goto exit;
			}

			mSimulateStep.ForEachCommandBuffer(clearBuffers);

			mEvaluateStep.GenerateCommandsFromEvaluations(mCurrentState, mSimulateStep);
			mCurrentState.Step(mSimulateStep);

			CE::Physics& physics = mCurrentState.GetWorld().GetPhysics();
			physics.UpdateBVHs({ .mForceRebuild = evaluateStepNum == 0 });
			physics.ResolveCollisions();

			if (mOnStepCompletedCallback)
			{
				CommandBuffer<MoveToCommand>& moveToBuffer = mSimulateStep.GetBuffer<MoveToCommand>();

				moveToBuffer.Clear();
				for (auto [entity, disk] : world.GetRegistry().View<CE::TransformedDiskColliderComponent>().each())
				{
					moveToBuffer.AddCommand(entity, disk.mCentre);
				}
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
