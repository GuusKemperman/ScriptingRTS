#include "Precomp.h"
#include "States/MoveToEntityState.h"

#include "Components/SimulationComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"
#include "Utilities/Math.h"

void RTS::MoveToEntityState::Execute(const GameState& gameState, GameSimulationStep& nextStep,
                                     std::span<const MoveToEntityState> commands)
{
	const CE::Registry& reg = gameState.GetWorld().GetRegistry();

	auto transformView = reg.View<CE::TransformedDiskColliderComponent>();
	CommandBuffer<MoveToCommand>& moveToBuffer = nextStep.GetBuffer<MoveToCommand>();

	for (const MoveToEntityState& moveState : commands)
	{
		if (!transformView.contains(moveState.mUnit)
			|| !transformView.contains(moveState.mTargetEntity))
		{
			continue;
		}

		const auto [unitTransform] = transformView.get(moveState.mUnit);
		const auto [targetTransform] = transformView.get(moveState.mTargetEntity);

		const glm::vec2 currentPosition = unitTransform.mCentre;
		const glm::vec2 targetPosition = targetTransform.mCentre;

		if (currentPosition == targetPosition)
		{
			continue;
		}

		const glm::vec2 delta = targetPosition - currentPosition;
		const glm::vec2 clampedDelta = CE::Math::ClampLength(delta, 0.0f, SimulationComponent::sSimulationStepSize * 5.0f);
		const glm::vec2 nextPosition = currentPosition + clampedDelta;

		moveToBuffer.AddCommand(moveState.mUnit, nextPosition);
	}
}

void RTS::MoveToEntityState::EnterState(const GameState&, 
	GameEvaluateStep& nextStep, 
	entt::entity unit,
	entt::entity targetEntity)
{
	nextStep.AddCommand(MoveToEntityState{ unit, targetEntity });
}
