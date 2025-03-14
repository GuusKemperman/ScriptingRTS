#include "Precomp.h"
#include "Core/UnitFilter.h"

#include <entt/entity/runtime_view.hpp>
#include <magic_enum/magic_enum_format.hpp>

#include "Components/TeamTag.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/RTSCollisionLayers.h"
#include "Meta/MetaProps.h"
#include "Meta/MetaReflect.h"
#include "Meta/MetaType.h"
#include "Meta/ReflectedTypes/ReflectEnums.h"
#include "World/Registry.h"
#include "World/World.h"
#include "World/Physics.h"

#ifdef EDITOR
void RTS::UnitFilter::DisplayWidget(const std::string& )
{
	CE::ShowInspectUI("Team", mTeam);
	CE::ShowInspectUI("SortByDistance", mSortByDistance);
}
#endif

entt::entity RTS::UnitFilter::operator()(const CE::World& world, entt::entity requestedByUnit) const
{
	const CE::Registry& reg = world.GetRegistry();
	entt::basic_runtime_view<const entt::sparse_set> view{};

	auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();

	if (diskStorage == nullptr)
	{
		return entt::null;
	}

	if (!diskStorage->contains(requestedByUnit))
	{
		LOG(LogGame, Error,
			"UnitFilter requested by entity {}, but entity has no position",
			entt::to_integral(requestedByUnit));
		return entt::null;
	}

	glm::vec2 requestPos = diskStorage->get(requestedByUnit).mCentre;

	view.iterate(*diskStorage);

	CE::CollisionRules collisionFilter{};
	collisionFilter.mLayer = CE::CollisionLayer::Query;

	if (mTeam == TeamFilter::Any)
	{
		collisionFilter.SetResponse(ToCE(CollisionLayer::Team1Layer), CE::CollisionResponse::Overlap);
		collisionFilter.SetResponse(ToCE(CollisionLayer::Team2Layer), CE::CollisionResponse::Overlap);
	}
	else
	{
		const TeamId* myTeamId = reg.TryGet<TeamId>(requestedByUnit);

		if (myTeamId == nullptr)
		{
			LOG(LogGame, Warning, 
				"TeamFilter was {}, but entity {} that performed this query did not belong to a team",
				mTeam,
				entt::to_integral(requestedByUnit));
			return entt::null;
		}

		TeamId targetTeamId = *myTeamId;

		if (mTeam != TeamFilter::Friendly)
		{
			targetTeamId = SwitchTeams(targetTeamId);
		}

		const CE::TypeId targetTeamTagId = GetTeamTagStorage(targetTeamId);

		const entt::sparse_set* storage = reg.Storage(targetTeamTagId);

		if (storage == nullptr)
		{
			return entt::null;
		}

		view.iterate(*storage);
		collisionFilter.SetResponse(ToCE(GetTeamLayer(targetTeamId)), CE::CollisionResponse::Overlap);
	}

	entt::entity bestEntity = entt::null;

	const CE::Physics::ExploreOrder order =
		[&]
		{
			switch (mSortByDistance)
			{
			case DistanceFilter::Nearest: return CE::Physics::ExploreOrder::NearestFirst;
			case DistanceFilter::Farthest: return CE::Physics::ExploreOrder::FarthestFirst;
			}
			ABORT;
			return CE::Physics::ExploreOrder::NearestFirst;
		}();

	world.GetPhysics().Explore(order,
		requestPos,
		collisionFilter,
		[&](entt::entity entity, [[maybe_unused]] float dist)
		{
			bestEntity = entity;
		},
		CE::Physics::ExploreDefaultShouldReturnFunction<true>{},
		[&](entt::entity entity)
		{
			return entity != requestedByUnit && view.contains(entity);
		});

	return bestEntity;
}

CE::MetaType RTS::UnitFilter::Reflect()
{
	CE::MetaType type = { CE::MetaType::T<UnitFilter>{}, "UnitFilter" };

	type.GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Add(CE::Props::sIsScriptOwnableTag);

	type.AddField(&UnitFilter::mTeam, "Team").GetProperties().Add(CE::Props::sIsScriptableTag);
	type.AddField(&UnitFilter::mSortByDistance, "SortByDistance").GetProperties().Add(CE::Props::sIsScriptableTag);

	CE::ReflectFieldType<UnitFilter>(type);

	return type;
}

CE::MetaType Reflector<RTS::TeamFilter>::Reflect()
{
	return CE::ReflectEnumType<RTS::TeamFilter>(true);
}

CE::MetaType Reflector<RTS::DistanceFilter>::Reflect()
{
	return CE::ReflectEnumType<RTS::DistanceFilter>(true);
}
