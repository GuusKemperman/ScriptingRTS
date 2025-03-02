#include "Precomp.h"
#include "Commands/SpawnUnitCommand.h"

#include "Components/TransformComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::SpawnUnitCommand::Execute(GameState& state, std::span<const SpawnUnitCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto& transformStorage = reg.Storage<CE::TransformComponent>();
	const CE::MetaType* playerScript = CE::MetaManager::Get().TryGetType("S_Unit");

	if (playerScript == nullptr)
	{
		LOG(LogGame, Error, "No player script!");
		return;
	}

	for (const SpawnUnitCommand& command : commands)
	{
		entt::entity entity = reg.Create();
		reg.AddComponent(*playerScript, entity);
		transformStorage.emplace(entity).SetWorldPosition(command.mPosition);
	}
}
