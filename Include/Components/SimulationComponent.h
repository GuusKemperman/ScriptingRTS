#pragma once
#include "Assets/Script.h"
#include "Assets/Core/AssetHandle.h"
#include "Core/GameState.h"
#include "Core/GameEvaluateStep.h"
#include "Core/GameSimulationStep.h"
#include "Meta/Fwd/MetaReflectFwd.h"
#include "Scripting/CompiledProgram.h"
#include "Utilities/ComponentFilter.h"

namespace CE
{
	class World;
}

namespace RTS
{
	struct OnSimulationStart :
		CE::EventType<OnSimulationStart>
	{
		OnSimulationStart() :
			EventType("OnSimulationStart")
		{
		}
	};
	inline OnSimulationStart sOnSimulationStart{};

	class SimulationComponent
	{
		void InvokeEvaluateEvents();

	public:
		~SimulationComponent();

		void OnBeginPlay(CE::World& world, entt::entity owner);

		void StartSimulation();

		void WaitForComplete();

		GameSimulationStep& GetNextSimulationStep() { return mSimulateStep; }
		const GameSimulationStep& GetNextSimulationStep() const { return mSimulateStep; }

		GameEvaluateStep& GetNextEvaluateStep() { return mEvaluateStep; }
		const GameEvaluateStep& GetNextEvaluateStep() const { return mEvaluateStep; }

		GameState& GetGameState() { return *mCurrentState; }
		const GameState& GetGameState() const { return *mCurrentState; }

		void OnPreEvaluate(entt::entity entity);

		void SetCompiledScripts(CompiledProgram team1, CompiledProgram team2);

		uint32 mStartingTotalNumOfUnits = 256;
		uint32 mNumStepsCompleted{};

		bool mShouldTeam1Start{};
		bool mUsePhysics = true;
		bool mUseMultiThreading = true;
		bool mRunOnCallingThread = false;

		CE::ComponentFilter mTeam1Script{};
		CE::ComponentFilter mTeam2Script{};

		std::function<void()> mInvokeEvaluateEvents = [this] { InvokeEvaluateEvents(); };
		std::function<void(const GameSimulationStep&)> mOnStepCompletedCallback{};

	private:
		void SimulateThread(const std::stop_token& stop);

		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(SimulationComponent);

		std::unique_ptr<GameState> mCurrentState{};
		GameSimulationStep mSimulateStep{};
		GameEvaluateStep mEvaluateStep{};

		std::jthread mThread{};
	};
}
