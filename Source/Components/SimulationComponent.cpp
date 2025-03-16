#include "Precomp.h"
#include "Components/SimulationComponent.h"

#include <entt/entity/runtime_view.hpp>

#include "Core/ScriptingAPI.h"
#include "Utilities/Reflect/ReflectComponentType.h"
#include "World/EventManager.h"
#include "World/Physics.h"

void RTS::SimulationComponent::OnBeginPlay(CE::World& viewportWorld, entt::entity)
{
	viewportWorld.GetEventManager().InvokeEventsForAllComponents(sOnSimulationStart);
	StartSimulation();
}

void RTS::SimulationComponent::StartSimulation()
{
	mCurrentState = std::make_unique<GameState>(mTeam1Script, mTeam2Script);

	if (mRunOnCallingThread)
	{
		SimulateThread({});
	}
	else
	{
		mThread = std::jthread{ [this](const std::stop_token& token) { SimulateThread(token); } };
	}
}

void RTS::SimulationComponent::WaitForComplete()
{
	mThread.join();
}

void RTS::SimulationComponent::OnPreEvaluate(entt::entity entity)
{
	Internal::OnUnitEvaluateTarget target{
	.sCurrentState = &GetGameState(),
	.sNextStep = &mEvaluateStep,
	.sCurrentUnit = entity
	};
	Internal::SetOnUnitEvaluateTargetForCurrentThread(target);
}

void RTS::SimulationComponent::SetCompiledScripts(CompiledProgram team1, CompiledProgram team2)
{
	mInvokeEvaluateEvents =
		[this, team1Program = std::move(team1), team2Program = std::move(team2)]
		{
			const CE::Registry& reg = GetGameState().GetWorld().GetRegistry();

			for (entt::entity entity : reg.View<Team1Tag>())
			{
				OnPreEvaluate(entity);
				team1Program.Run();
			}

			for (entt::entity entity : reg.View<Team2Tag>())
			{
				OnPreEvaluate(entity);
				team2Program.Run();
			}
		};
}

void RTS::SimulationComponent::InvokeEvaluateEvents()
{
	CE::World& world = GetGameState().GetWorld();
	for (const CE::BoundEvent& boundEvent : world.GetEventManager().GetBoundEvents(sOnUnitEvaluate))
	{
		const CE::TypeId typeId = boundEvent.mType.get().GetTypeId();
		entt::sparse_set* const storage = world.GetRegistry().Storage(typeId);

		if (storage == nullptr)
		{
			continue;
		}

		const CE::MetaFunc& func = boundEvent.mFunc.get();
		static constexpr std::array argForms = { CE::TypeForm::Ref, CE::TypeForm::Ref, CE::TypeForm::ConstRef };

		entt::runtime_view view{};
		view.iterate(*storage);

		bool isStatic = func.GetParameters().at(0).mTypeTraits.mStrippedTypeId != typeId;

		auto evaluateEntity = 
			[&](entt::entity entity)
			{
				std::array args{
					!isStatic ? CE::MetaAny{ boundEvent.mType, storage->value(entity), false } : CE::MetaAny{ boundEvent.mType },
					CE::MetaAny{ world },
					CE::MetaAny{ entity }
				};

				OnPreEvaluate(entity);

				func.InvokeUnchecked(std::span{ args.data() + isStatic, static_cast<size_t>(3 - isStatic) }, 
					std::span{ argForms.data() + isStatic, static_cast<size_t>(3 - isStatic) });
			};

		if (mUseMultiThreading)
		{
			std::for_each(std::execution::par_unseq, view.begin(), view.end(), evaluateEntity);
		}
		else
		{
			std::for_each(view.begin(), view.end(), evaluateEntity);
		}
	}
}

RTS::SimulationComponent::~SimulationComponent()
{

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

		for (uint32 x = 0, numSpawned = 0; numSpawned < mStartingTotalNumOfUnits / 2; x++)
		{
			for (uint32 y = 0; y < std::max(static_cast<uint32>(sqrtf(static_cast<float>(mStartingTotalNumOfUnits / 2))), 1u) 
					&& numSpawned < mStartingTotalNumOfUnits / 2; y++, numSpawned++)
			{
				const glm::vec2 pos1 = glm::vec2{ .9f } * static_cast<glm::vec2>(glm::ivec2{ 10 } + glm::ivec2{ x, y });
				const glm::vec2 pos2 = -pos1;

				if (mShouldTeam1Start)
				{
					mSimulateStep.AddCommand(SpawnUnitCommand{ pos1, TeamId::Team1 , UnitType::Tank });
					mSimulateStep.AddCommand(SpawnUnitCommand{ pos2, TeamId::Team2 , UnitType::Tank });
				}
				else
				{
					mSimulateStep.AddCommand(SpawnUnitCommand{ pos2, TeamId::Team2 , UnitType::Tank });
					mSimulateStep.AddCommand(SpawnUnitCommand{ pos1, TeamId::Team1 , UnitType::Tank });
				}
			}
		}

		GetGameState().Step(mSimulateStep);
		mNumStepsCompleted++;

		if (mOnStepCompletedCallback)
		{
			mOnStepCompletedCallback(mSimulateStep);
		}
	}

	if (stop.stop_requested())
	{
		goto exit;
	}

	for (int evaluateStepNum = 0; evaluateStepNum < Constants::sTotalNumEvaluateSteps; evaluateStepNum++)
	{
		CE::World& world = GetGameState().GetWorld();

		CE::Physics& physics = GetGameState().GetWorld().GetPhysics();
		physics.UpdateBVHs({ .mForceRebuild = evaluateStepNum == 0 });

		mEvaluateStep.ForEachCommandBuffer(clearBuffers);

		mInvokeEvaluateEvents();

		for (int simulateStepNum = 0; simulateStepNum < Constants::sNumSimulationStepsBetweenEvaluate; simulateStepNum++)
		{
			if (stop.stop_requested())
			{
				goto exit;
			}

			mSimulateStep.ForEachCommandBuffer(clearBuffers);

			if (stop.stop_requested())
			{
				goto exit;
			}

			if (simulateStepNum != 0)
			{
				physics.UpdateBVHs();
			}

			if (stop.stop_requested())
			{
				goto exit;
			}

			if (mUsePhysics)
			{
				physics.ResolveCollisions();
			}

			if (stop.stop_requested())
			{
				goto exit;
			}

			mEvaluateStep.GenerateCommandsFromEvaluations(GetGameState(), mSimulateStep);

			if (stop.stop_requested())
			{
				goto exit;
			}

			GetGameState().Step(mSimulateStep);

			if (stop.stop_requested())
			{
				goto exit;
			}

			mNumStepsCompleted++;

			if (stop.stop_requested())
			{
				goto exit;
			}

			if (mOnStepCompletedCallback)
			{
				CommandBuffer<MoveToCommand>& moveToBuffer = mSimulateStep.GetBuffer<MoveToCommand>();

				moveToBuffer.Clear();
				for (auto [entity, disk] : world.GetRegistry().View<CE::TransformedDiskColliderComponent>().each())
				{
					moveToBuffer.AddCommand(entity, disk.mCentre);
				}

				if (stop.stop_requested())
				{
					goto exit;
				}

				mOnStepCompletedCallback(mSimulateStep);
			}

			if (GetGameState().IsGameOver())
			{
				goto exit;
			}
		}
	}
exit:;
}

CE::MetaType RTS::SimulationComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<SimulationComponent>{}, "SimulationComponent" };

	metaType.AddField(&SimulationComponent::mStartingTotalNumOfUnits, "mStartingTotalNumOfUnits");
	metaType.AddField(&SimulationComponent::mTeam1Script, "mTeam1Script");
	metaType.AddField(&SimulationComponent::mTeam2Script, "mTeam2Script");
	metaType.AddField(&SimulationComponent::mShouldTeam1Start, "mShouldTeam1Start");
	metaType.AddField(&SimulationComponent::mUsePhysics, "mUsePhysics");
	metaType.AddField(&SimulationComponent::mUseMultiThreading, "mUseMultiThreading");
	metaType.AddField(&SimulationComponent::mRunOnCallingThread, "mRunOnCallingThread");
	metaType.AddField(&SimulationComponent::mNumStepsCompleted, "mNumStepsCompleted")
		.GetProperties().Add(CE::Props::sIsEditorReadOnlyTag);

	CE::BindEvent(metaType, CE::sOnBeginPlay, &SimulationComponent::OnBeginPlay);

	CE::ReflectComponentType<SimulationComponent>(metaType);

	return metaType;
}
