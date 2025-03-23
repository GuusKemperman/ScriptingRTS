#include "Precomp.h"
#include "Commands/SpawnObjectiveCommand.h"

#include "Components/ObjectiveComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Components/Physics2D/PhysicsBody2DComponent.h"
#include "Core/GameState.h"
#include "Core/RTSCollisionLayers.h"
#include "Core/WeaponTypes.h"
#include "World/Registry.h"

void RTS::SpawnObjectiveCommand::Execute(GameState& state, 
                                         std::span<const SpawnObjectiveCommand> commands)
{
	for (const auto [position] : commands)
	{
		CE::Registry& reg = state.GetWorld().GetRegistry();
		entt::entity entity = reg.Create();

		reg.AddComponent<CE::TransformedDiskColliderComponent>(entity, 
			position, 
			sWeaponTypes[0].mRangeDistance.mNamedRanges.mShortRangeEnd);

		CE::CollisionRules& rules = reg.AddComponent<CE::PhysicsBody2DComponent>(entity).mRules;
		rules.mLayer = ToCE(CollisionLayer::Objectives);
		rules.SetResponse(CE::CollisionLayer::Query, CE::CollisionResponse::Overlap);

		reg.AddComponent<TeamId>(entity, TeamId::Neutral);
		reg.AddComponent<NeutralTag>(entity);

		reg.AddComponent<ObjectiveComponent>(entity);

		reg.AddComponent<EntityType::Enum>(entity, EntityType::Objective);
		reg.AddComponent<ObjectiveTag>(entity);
	}
}
