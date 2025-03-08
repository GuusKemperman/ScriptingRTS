#pragma once
#include "Assets/Script.h"
#include "Assets/Core/AssetHandle.h"
#include "Core/GameState.h"
#include "Core/GameEvaluateStep.h"
#include "Core/GameSimulationStep.h"
#include "Meta/Fwd/MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace RTS
{
	class SimulationComponent
	{
	public:
		void OnBeginPlay(CE::World& world, entt::entity owner);

		void StartSimulation(std::function<void(const GameSimulationStep&)> onStepCompleted = {});

		void WaitForComplete();

		GameSimulationStep& GetNextSimulationStep() { return mSimulateStep; }
		const GameSimulationStep& GetNextSimulationStep() const { return mSimulateStep; }

		GameEvaluateStep& GetNextEvaluateStep() { return mEvaluateStep; }
		const GameEvaluateStep& GetNextEvaluateStep() const { return mEvaluateStep; }

		GameState& GetGameState() { return mCurrentState; }
		const GameState& GetGameState() const { return mCurrentState; }

		uint32 mStartingTotalNumOfUnits = 2;

	private:

		void InvokeEvaluateEvents();
		void SimulateThread(const std::stop_token& stop);

		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(SimulationComponent);

		GameState mCurrentState{};
		GameSimulationStep mSimulateStep{};
		GameEvaluateStep mEvaluateStep{};

		std::function<void(const GameSimulationStep&)> mOnStepCompletedCallback{};

		std::jthread mThread{};
	};
}
