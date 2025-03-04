#include "Precomp.h"
#include "Commands/MoveToCommand.h"

#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

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
