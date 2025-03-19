#include "Precomp.h"
#include "States/FleeFromEntityState.h"

#include "Components/SimulationComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::FleeFromEntityState::Execute(const GameState& gameState, GameSimulationStep& nextStep,
	std::span<const FleeFromEntityState> commands)
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
			if (!diskStorage->contains(target)
				|| !unitStorage->contains(unit)
				|| !diskStorage->contains(unit))
			{
				return;
			}

			const glm::vec2 currentPosition = diskStorage->get(unit).mCentre;
			const glm::vec2 fleeFromPosition = diskStorage->get(target).mCentre;

			// Can't flee without losing symmetry
			if (currentPosition == fleeFromPosition)
			{
				return;
			}

			const float movementSpeed = GetUnitType(unitStorage->get(unit)).mMovementSpeed;
			const glm::vec2 direction = glm::normalize(currentPosition - fleeFromPosition);
			const glm::vec2 nextPosition = currentPosition + direction * Constants::sSimulationStepSize * movementSpeed;

			moveToBuffer.AddCommand(unit, nextPosition);
		});
}

