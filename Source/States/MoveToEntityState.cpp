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

	ForEach(gameState, commands, 
		[&](entt::entity unit, entt::entity target)
		{
			if (!diskStorage->contains(target))
			{
				return;
			}

			MoveToCommand::AddMoveTowardsPositionCommand(*diskStorage,
				*unitStorage,
				moveToBuffer,
				unit,
				diskStorage->get(target).mCentre);
		});
}

