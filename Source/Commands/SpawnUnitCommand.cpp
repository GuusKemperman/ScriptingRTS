#include "Precomp.h"
#include "Commands/SpawnUnitCommand.h"

#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::SpawnUnitCommand::Execute(GameState& state, std::span<const SpawnUnitCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto& transformStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto& physicsStorage = reg.Storage<CE::PhysicsBody2DComponent>();

	const CE::MetaType* playerScript = CE::MetaManager::Get().TryGetType("S_Unit");

	if (playerScript == nullptr)
	{
		LOG(LogGame, Error, "No player script!");
		return;
	}

	CE::CollisionRules rules{};
	rules.mLayer = CE::CollisionLayer::Character;
	rules.SetResponse(CE::CollisionLayer::Character, CE::CollisionResponse::Blocking);

	for (const SpawnUnitCommand& command : commands)
	{
		entt::entity entity = reg.Create();
		reg.AddComponent(*playerScript, entity);

		physicsStorage.emplace(entity).mRules = rules;
		transformStorage.emplace(entity, command.mPosition, 1.0f);
	}
}
