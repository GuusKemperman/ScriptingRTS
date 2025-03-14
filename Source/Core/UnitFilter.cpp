#include "Precomp.h"
#include "Core/UnitFilter.h"

#include <entt/entity/runtime_view.hpp>
#include <magic_enum/magic_enum_format.hpp>

#include "Components/TeamTag.h"
#include "Components/WeaponComponent.h"
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
	CE::ShowInspectUI("Range", mRange);
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
			case SortPriority::Nearest: return CE::Physics::ExploreOrder::NearestFirst;
			case SortPriority::Farthest: return CE::Physics::ExploreOrder::FarthestFirst;
			}
			ABORT;
			return CE::Physics::ExploreOrder::NearestFirst;
		}();

	const auto [minRange, maxRange] = [&]() -> std::pair<float, float>
		{
			if (mRange == RangeFilter::Any)
			{
				return { -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
			}

			const WeaponComponent* weapon = reg.TryGet<WeaponComponent>(requestedByUnit);

			if (weapon == nullptr)
			{
				return { -std::numeric_limits<float>::infinity(), std::numeric_limits<float>::infinity() };
			}

			const WeaponType& weaponType = GetWeaponType(weapon->mType);

			const auto& range = weaponType.mRange.mNamedRanges;

			switch (mRange)
			{
			case RangeFilter::InShortRange:		return { 0.0f, range.mShortRangeEnd }; 
			case RangeFilter::WithinShortRange:	return { 0.0f, range.mShortRangeEnd };
			case RangeFilter::OutShortRange:	return { range.mShortRangeEnd, std::numeric_limits<float>::infinity() };
			case RangeFilter::InMidRange:		return { range.mShortRangeEnd, range.mMidRangeEnd };
			case RangeFilter::WithinMidRange:	return { 0.0f, range.mMidRangeEnd };
			case RangeFilter::OutMidRange:		return { range.mMidRangeEnd, std::numeric_limits<float>::infinity() };
			case RangeFilter::InLongRange:		return { range.mMidRangeEnd, range.mLongRangeEnd }; 
			case RangeFilter::WithinLongRange:	return { 0.0f, range.mLongRangeEnd }; 
			case RangeFilter::OutLongRange:		return { range.mLongRangeEnd, std::numeric_limits<float>::infinity() };
			}
			ABORT;
			return {};
		}();

	world.GetPhysics().Explore(order,
		requestPos,
		collisionFilter,
		[&](entt::entity entity, float dist)
		{
			if (dist >= minRange
				&& dist <= maxRange)
			{
				bestEntity = entity;
			}
		},
		[&](entt::entity, float dist)
		{
			return bestEntity != entt::null
				|| (order == CE::Physics::ExploreOrder::NearestFirst && dist > maxRange)
				|| (order == CE::Physics::ExploreOrder::FarthestFirst && dist < minRange);
		},
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
	type.AddField(&UnitFilter::mRange, "Range").GetProperties().Add(CE::Props::sIsScriptableTag);

	CE::ReflectFieldType<UnitFilter>(type);

	return type;
}

CE::MetaType Reflector<RTS::TeamFilter>::Reflect()
{
	return CE::ReflectEnumType<RTS::TeamFilter>(true);
}

CE::MetaType Reflector<RTS::SortPriority>::Reflect()
{
	return CE::ReflectEnumType<RTS::SortPriority>(true);
}

CE::MetaType Reflector<RTS::RangeFilter>::Reflect()
{
	return CE::ReflectEnumType<RTS::RangeFilter>(true);
}
