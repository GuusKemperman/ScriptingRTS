#include "Precomp.h"
#include "Core/EntityFilter.h"

#include <bitset>
#include <entt/entity/runtime_view.hpp>
#include <magic_enum/magic_enum_format.hpp>

#include "Components/ObjectiveComponent.h"
#include "Components/TeamTag.h"
#include "Components/WeaponComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "Core/RTSCollisionLayers.h"
#include "Core/UnitTypes.h"
#include "Meta/MetaProps.h"
#include "Meta/MetaReflect.h"
#include "Meta/MetaType.h"
#include "Meta/ReflectedTypes/ReflectEnums.h"
#include "World/Registry.h"
#include "World/World.h"
#include "World/Physics.h"

#ifdef EDITOR
void RTS::EntityFilter::DisplayWidget(const std::string& )
{
	CE::ShowInspectUI("SortByDistance", mSortByDistance);

	CE::ShowInspectUI("Type", mType.mEnum);
	CE::ShowInspectUI("Team", mTeam.mEnum);
	CE::ShowInspectUI("Range", mRange.mEnum);
	CE::ShowInspectUI("Health", mHealth.mEnum);

	ImGui::TextUnformatted("Better script viewing coming soon!");
}
#endif

entt::entity RTS::EntityFilter::operator()(const GameState& state, entt::entity requestedByUnit) const
{
	const CE::Registry& reg = state.GetWorld().GetRegistry();

	auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();

	if (diskStorage == nullptr)
	{
		return entt::null;
	}

	if (!diskStorage->contains(requestedByUnit))
	{
		LOG(LogGame, Error,
			"EntityFilter requested by entity {}, but entity has no position",
			entt::to_integral(requestedByUnit));
		return entt::null;
	}

	const glm::vec2 requestPos = diskStorage->get(requestedByUnit).mCentre;

	CE::CollisionRules collisionFilter{};
	collisionFilter.mLayer = CE::CollisionLayer::Query;

	// TODO only overlap desired layers
	collisionFilter.SetResponse(ToCE(CollisionLayer::Team1Layer), CE::CollisionResponse::Overlap);
	collisionFilter.SetResponse(ToCE(CollisionLayer::Team2Layer), CE::CollisionResponse::Overlap);
	collisionFilter.SetResponse(ToCE(CollisionLayer::Objectives), CE::CollisionResponse::Overlap);

	const MakeCacheParam makeCacheParam{ state, *this, requestedByUnit };
	TypeFilter::PerEntityCache perEntityCacheType{ makeCacheParam };
	TeamFilter::PerEntityCache perEntityCacheTeam{ makeCacheParam };
	RangeFilter::PerEntityCache perEntityCacheRange{ makeCacheParam };
	HealthFilter::PerEntityCache perEntityCacheHealth{ makeCacheParam };

	entt::entity bestEntity = entt::null;

	auto isValidEntity = [&](entt::entity entity)
		{
			const CheckEntityParam checkEntityParam{ state, *this, requestedByUnit, entity };

			return mType(checkEntityParam, perEntityCacheType)
				&& mTeam(checkEntityParam, perEntityCacheTeam)
				&& mRange(checkEntityParam, perEntityCacheRange)
				&& mHealth(checkEntityParam, perEntityCacheHealth);
		};

	auto selectEntity = [&](entt::entity entity, [[maybe_unused]] float dist)
		{
			bestEntity = entity;
		};

	const CE::Physics::ExploreOrder order =
		[&]
		{
			switch (mSortByDistance)
			{
			case SortPriority::Nearest: return CE::Physics::ExploreOrder::NearestFirst;
			case SortPriority::Farthest: return CE::Physics::ExploreOrder::FarthestFirst;
			}
			ABORT;
			return CE::Physics::ExploreOrder::NearestFirst;
		}();

	state.GetWorld().GetPhysics().Explore(order,
		requestPos,
		collisionFilter,
		selectEntity,
		[&](entt::entity, [[maybe_unused]] float dist)
		{
			return bestEntity != entt::null;
		},
		isValidEntity);

	return bestEntity;
}

CE::MetaType RTS::EntityFilter::Reflect()
{
	CE::MetaType type = { CE::MetaType::T<EntityFilter>{}, "EntityFilter" };

	type.GetProperties()
		.Add(CE::Props::sIsScriptableTag)
		.Add(CE::Props::sIsScriptOwnableTag);

	CE::ReflectFieldType<EntityFilter>(type);

	return type;
}

CE::MetaType Reflector<RTS::SortPriority>::Reflect()
{
	return CE::ReflectEnumType<RTS::SortPriority>(true);
}

