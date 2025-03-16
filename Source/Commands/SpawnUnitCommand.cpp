#include "Precomp.h"
#include "Commands/SpawnUnitCommand.h"

#include "Components/HealthComponent.h"
#include "Components/UnitTypeTag.h"
#include "Components/WeaponComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"
#include "Core/GameState.h"
#include "Core/RTSCollisionLayers.h"
#include "Utilities/Random.h"
#include "World/Registry.h"

void RTS::SpawnUnitCommand::Execute(GameState& state, std::span<const SpawnUnitCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto& transformStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto& physicsStorage = reg.Storage<CE::PhysicsBody2DComponent>();

	auto& weaponStorage = reg.Storage<WeaponComponent>();
	auto& healthStorage = reg.Storage<HealthComponent>();

	auto& rngStorage = reg.Storage<CE::DefaultRandomEngine>();

	auto& teamStorage = reg.Storage<TeamId>();
	auto& team1Storage = reg.Storage<Team1Tag>();
	auto& team2Storage = reg.Storage<Team2Tag>();

	auto& unitStorage = reg.Storage<UnitType::Enum>();
	auto& tankStorage = reg.Storage<TankTag>();

	CE::CollisionRules rules{};
	rules.SetResponse(CE::CollisionLayer::Query, CE::CollisionResponse::Overlap);
	rules.SetResponse(ToCE(CollisionLayer::Team1Layer), CE::CollisionResponse::Blocking);
	rules.SetResponse(ToCE(CollisionLayer::Team2Layer), CE::CollisionResponse::Blocking);

	for (const SpawnUnitCommand& command : commands)
	{
		const UnitType type = GetUnitType(command.mUnitType);

		entt::entity entity = reg.Create();

		CE::ComponentFilter script = state.GetTeamScript(command.mTeamId);

		if (script != nullptr)
		{
			reg.AddComponent(*script, entity);
		}

		transformStorage.emplace(entity, command.mPosition, type.mRadius);
		unitStorage.emplace(entity, command.mUnitType);
		teamStorage.emplace(entity, command.mTeamId);

		switch (command.mTeamId)
		{
		case TeamId::Team1:
		{
			team1Storage.emplace(entity);
			rules.mLayer = CE::CollisionLayer::GameLayer0;
			break;
		}
		case TeamId::Team2:
		{
			team2Storage.emplace(entity);
			rules.mLayer = CE::CollisionLayer::GameLayer1;
			break;
		}
		default: ABORT;
		}

		rngStorage.emplace(entity);
		physicsStorage.emplace(entity).mRules = rules;

		switch (command.mUnitType)
		{
		case UnitType::Tank: tankStorage.emplace(entity); break;
		default: ABORT;
		}

		weaponStorage.emplace(entity, type.mWeaponType);
		healthStorage.emplace(entity, type.mHealth);
	}
}
