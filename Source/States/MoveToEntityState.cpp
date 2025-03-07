#include "Precomp.h"
#include "States/MoveToEntityState.h"

#include "Components/SimulationComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::MoveToEntityState::Execute(const GameState& gameState, GameSimulationStep& nextStep,
                                     std::span<const MoveToEntityState> commands)
{
	const CE::Registry& reg = gameState.GetWorld().GetRegistry();

	auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto* unitStorage = reg.Storage<UnitType::Enum>();

	if (diskStorage == nullptr
		|| unitStorage == nullptr)
	{
		return;
	}

	CommandBuffer<MoveToCommand>& moveToBuffer = nextStep.GetBuffer<MoveToCommand>();

	for (const MoveToEntityState& moveState : commands)
	{
		if (!diskStorage->contains(moveState.mTargetEntity))
		{
			continue;
		}

		MoveToCommand::AddMoveTowardsPositionCommand(*diskStorage, 
			*unitStorage,
			moveToBuffer,
			moveState.mUnit, 
			diskStorage->get(moveState.mTargetEntity).mCentre);
	}
}

void RTS::MoveToEntityState::EnterState(const GameState&, 
	GameEvaluateStep& nextStep, 
	entt::entity unit,
	entt::entity targetEntity)
{
	nextStep.AddCommand(MoveToEntityState{ unit, targetEntity });
}
