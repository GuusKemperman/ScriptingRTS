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
		static constexpr uint32 sNumSimulationStepsBetweenEvaluate = 10;
		static constexpr float sSimulationStepSize = .1f;
		static constexpr float sSimulationDurationSeconds = 3600.0f;

		static constexpr float sEvaluateStepSize = sNumSimulationStepsBetweenEvaluate * sSimulationStepSize;
		static constexpr uint32 sTotalNumEvaluateSteps = static_cast<uint32>(sSimulationDurationSeconds / sEvaluateStepSize);
		static constexpr uint32 sTotalNumSimulationSteps = static_cast<uint32>(sSimulationDurationSeconds / sSimulationStepSize);

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
