#include "Precomp.h"
#include "Commands/SpawnUnitCommand.h"

#include "Components/UnitTypeTag.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::SpawnUnitCommand::Execute(GameState& state, std::span<const SpawnUnitCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto& transformStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto& physicsStorage = reg.Storage<CE::PhysicsBody2DComponent>();

	auto& teamStorage = reg.Storage<TeamId>();
	auto& team1Storage = reg.Storage<Team1Tag>();
	auto& team2Storage = reg.Storage<Team2Tag>();

	auto& unitStorage = reg.Storage<UnitType::Enum>();

	auto& tankStorage = reg.Storage<TankTag>();

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
		transformStorage.emplace(entity, command.mPosition, GetUnitProperty<&UnitType::mRadius>(command.mUnitType));

		unitStorage.emplace(entity, command.mUnitType);

		teamStorage.emplace(entity, command.mTeamId);

		switch (command.mTeamId)
		{
		case TeamId::Team1:	team1Storage.emplace(entity); break;
		case TeamId::Team2:	team2Storage.emplace(entity); break;
		default: ABORT;
		}

		switch (command.mUnitType)
		{
		case UnitType::Tank: tankStorage.emplace(entity); break;
		default: ABORT;
		}
	}
}
