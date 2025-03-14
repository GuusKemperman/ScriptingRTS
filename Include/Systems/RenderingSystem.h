#pragma once
#include "Systems/System.h"
#include "Assets/Material.h"
#include "Assets/StaticMesh.h"
#include "Assets/Core/AssetHandle.h"
#include "Core/GameState.h"
#include "Core/GameSimulationStep.h"

namespace RTS
{
	class RenderingSystem :
		public CE::System
	{
	public:
		RenderingSystem();

		void RecordStep(const GameSimulationStep& step);

		void Update(CE::World& world, float dt) override;

		void Render(const CE::World& world, CE::RenderCommandQueue& renderQueue) const override;

	private:
		void StepToCorrectGameState(CE::World& world, float dt);
		void UpdateCameraMovement(CE::World& world, float dt) const;

		CE::AssetHandle<CE::Material> mMat{};
		CE::AssetHandle<CE::StaticMesh> mMesh{};

		mutable std::mutex mRenderingQueueMutex{};
		std::vector<GameSimulationStep> mRenderingQueue{};

		std::unique_ptr<GameState> mPreviousState{};
		std::unique_ptr<GameState> mCurrentState{};
	};
}
