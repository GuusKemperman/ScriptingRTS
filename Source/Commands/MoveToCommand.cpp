#include "Precomp.h"
#include "Commands/MoveToCommand.h"

#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"
#include "Components/SimulationComponent.h"
#include "Utilities/Math.h"

void RTS::MoveToCommand::Execute(GameState& state, std::span<const MoveToCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto transformView = reg.View<CE::TransformedDiskColliderComponent>();

	for (const MoveToCommand& moveCommand : commands)
	{
		auto [transform] = transformView.get(moveCommand.mUnit);
		transform.mCentre = moveCommand.mPosition;
	}
}

void RTS::MoveToCommand::AddMoveTowardsPositionCommand(
	const entt::storage_for_t<CE::TransformedDiskColliderComponent>& transformStorage,
	const entt::storage_for_t<UnitType::Enum>& unitStorage,
	CommandBuffer<MoveToCommand>& moveCommandBuffer,
	entt::entity unit,
	glm::vec2 targetPosition)
{
	if (!unitStorage.contains(unit))
	{
		return;
	}

	const CE::TransformedDiskColliderComponent& unitTransform = transformStorage.get(unit);
	const glm::vec2 currentPosition = unitTransform.mCentre;

	if (currentPosition == targetPosition)
	{
		return;
	}

	const float movementSpeed = GetUnitType(unitStorage.get(unit)).mMovementSpeed;
	const glm::vec2 delta = targetPosition - currentPosition;
	const glm::vec2 clampedDelta = CE::Math::ClampLength(delta, 0.0f, Constants::sSimulationStepSize * movementSpeed);
	const glm::vec2 nextPosition = currentPosition + clampedDelta;

	moveCommandBuffer.AddCommand(unit, nextPosition);
}