#include "Precomp.h"
#include "States/MoveToPositionState.h"

#include "Commands/MoveToCommand.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameEvaluateStep.h"
#include "Core/GameSimulationStep.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::MoveToPositionState::Execute(const GameState& gameState, GameSimulationStep& nextStep,
                                       std::span<const MoveToPositionState> commands)
{
	const CE::Registry& reg = gameState.GetWorld().GetRegistry();

	auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();

	if (diskStorage == nullptr)
	{
		return;
	}

	CommandBuffer<MoveToCommand>& moveToBuffer = nextStep.GetBuffer<MoveToCommand>();

	for (const MoveToPositionState& moveState : commands)
	{
		MoveToCommand::AddMoveTowardsPositionCommand(*diskStorage,
			moveToBuffer,
			moveState.mUnit,
			moveState.mTargetPosition);
	}
}

void RTS::MoveToPositionState::EnterState(const GameState&,
	GameEvaluateStep& nextStep,
	entt::entity unit,
	glm::vec2 targetPosition)
{
	nextStep.AddCommand(MoveToPositionState{ unit, targetPosition });
}
