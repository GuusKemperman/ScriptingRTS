#include "Precomp.h"
#include "Core/UnitFilter.h"

#include <entt/entity/runtime_view.hpp>
#include <magic_enum/magic_enum_format.hpp>

#include "Components/HealthComponent.h"
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
void RTS::UnitFilter::DisplayWidget(const std::string& )
{
	CE::ShowInspectUI("Team", mTeam);
	CE::ShowInspectUI("SortByDistance", mSortByDistance);
	CE::ShowInspectUI("Range", mRange);
	CE::ShowInspectUI("Health", mHealth);
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

		if (mTeam == TeamFilter::Enemy)
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
			case RangeFilter::InShortRange:		return { -std::numeric_limits<float>::infinity(), range.mShortRangeEnd };
			case RangeFilter::InShortRangeOrCloser:	return { -std::numeric_limits<float>::infinity(), range.mShortRangeEnd };
			case RangeFilter::OutShortRange:	return { range.mShortRangeEnd, std::numeric_limits<float>::infinity() };
			case RangeFilter::InMidRange:		return { range.mShortRangeEnd, range.mMidRangeEnd };
			case RangeFilter::InMidRangeOrCloser:	return { -std::numeric_limits<float>::infinity(), range.mMidRangeEnd };
			case RangeFilter::OutMidRange:		return { range.mMidRangeEnd, std::numeric_limits<float>::infinity() };
			case RangeFilter::InLongRange:		return { range.mMidRangeEnd, range.mLongRangeEnd }; 
			case RangeFilter::InLongRangeOrCloser:	return { -std::numeric_limits<float>::infinity(), range.mLongRangeEnd };
			case RangeFilter::OutLongRange:		return { range.mLongRangeEnd, std::numeric_limits<float>::infinity() };
			}
			ABORT;
			return {};
		}();

	struct HealthCheck
	{
		HealthCheck(HealthFilter mHealth) :
			mHealthFilter(mHealth)
		{
			if (mHealth == HealthFilter::Any)
			{
				return;
			}

			mHealthStorage = reg.Storage<HealthComponent>();
			mUnitTypeStorage = reg.Storage<UnitType::Enum>();
			ASSERT(mHealthStorage != nullptr && mUnitTypeStorage != nullptr);
		}

		bool operator()(entt::entity entity) const
		{
			if (mHealthFilter == HealthFilter::Any)
			{
				return true;
			}

			if (!mUnitTypeStorage->contains(entity)
				|| !mHealthStorage->contains(entity))
			{
				return false;
			}

			const UnitType unitType = GetUnitType(mUnitTypeStorage->get(entity));
			const float unitHealth = mHealthStorage->get(entity).mHealth;
			const float healthPercentage = unitHealth / unitType.mHealth;

			switch (mHealthFilter)
			{
			case HealthFilter::Below25: return healthPercentage <  0.25f;
			case HealthFilter::Above25: return healthPercentage >= 0.25f;
			case HealthFilter::Below50: return healthPercentage <  0.50f;
			case HealthFilter::Above50: return healthPercentage >= 0.50f;
			case HealthFilter::Below75: return healthPercentage <  0.75f;
			case HealthFilter::Above75: return healthPercentage >= 0.75f;
			case HealthFilter::Below100:return healthPercentage <  1.00f;
			case HealthFilter::Full:	return healthPercentage >= 1.00f;
			}

			ABORT;
			return false;
		}

		HealthFilter mHealthFilter{};
		const entt::storage_for_t<HealthComponent>* mHealthStorage{};
		const entt::storage_for_t<UnitType::Enum>* mUnitTypeStorage{};
	} healthCheck{ mHealth };

	auto isValidEntity = [&](entt::entity entity)
		{
			return entity != requestedByUnit
					&& view.contains(entity)
					&& healthCheck(entity);
		};

	auto selectEntity = [&](entt::entity entity, float dist)
		{
			if (dist >= minRange
				&& dist <= maxRange)
			{
				bestEntity = entity;
			}
		};

	if (mTeam == TeamFilter::Myself)
	{
		if (isValidEntity(requestedByUnit))
		{
			selectEntity(requestedByUnit, 0.0f);
		}
		return bestEntity;
	}

	world.GetPhysics().Explore(order,
		requestPos,
		collisionFilter,
		selectEntity,
		[&](entt::entity, float dist)
		{
			return bestEntity != entt::null
				|| (order == CE::Physics::ExploreOrder::NearestFirst && dist > maxRange)
				|| (order == CE::Physics::ExploreOrder::FarthestFirst && dist < minRange);
		},
		isValidEntity);

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

CE::MetaType Reflector<RTS::HealthFilter>::Reflect()
{
	return CE::ReflectEnumType<RTS::HealthFilter>(true);
}
