#include "Precomp.h"
#include "Core/UnitFilter.h"

#include <entt/entity/runtime_view.hpp>
#include <magic_enum/magic_enum_format.hpp>

#include "Components/TeamTag.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Meta/MetaProps.h"
#include "Meta/MetaReflect.h"
#include "Meta/MetaType.h"
#include "Meta/ReflectedTypes/ReflectEnums.h"
#include "World/Registry.h"
#include "World/World.h"

#ifdef EDITOR
void RTS::UnitFilter::DisplayWidget(const std::string& )
{
	CE::ShowInspectUI("Team", mTeam);
	CE::ShowInspectUI("SortByDistance", mSortByDistance);
}
#endif

entt::entity RTS::UnitFilter::operator()(const CE::World& world) const
{
	const CE::Registry& reg = world.GetRegistry();
	entt::basic_runtime_view<const entt::sparse_set> view{};

	auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();

	if (diskStorage == nullptr)
	{
		return entt::null;
	}

	if (!diskStorage->contains(mRequestedByUnit))
	{
		LOG(LogGame, Error,
			"UnitFilter requested by entity {}, but entity has no position",
			entt::to_integral(mRequestedByUnit));
		return entt::null;
	}

	glm::vec2 requestPos = diskStorage->get(mRequestedByUnit).mCentre;

	view.iterate(*diskStorage);

	if (mTeam != TeamFilter::Any)
	{
		const TeamId* myTeamId = reg.TryGet<TeamId>(mRequestedByUnit);

		if (myTeamId == nullptr)
		{
			LOG(LogGame, Warning, 
				"TeamFilter was {}, but entity {} that performed this query did not belong to a team",
				mTeam,
				entt::to_integral(mRequestedByUnit));
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
	}

	float bestDist2 = mSortByDistance == DistanceFilter::Nearest ?
		std::numeric_limits<float>::infinity() :
		-std::numeric_limits<float>::infinity();
	entt::entity bestEntity = entt::null;

	for (entt::entity entity : view)
	{
		if (entity == mRequestedByUnit)
		{
			continue;
		}

		CE::TransformedDiskColliderComponent disk = diskStorage->get(entity);

		float dist2 = glm::distance2(disk.mCentre, requestPos);

		if ((mSortByDistance == DistanceFilter::Nearest && dist2 < bestDist2)
			|| (mSortByDistance == DistanceFilter::Furthest && dist2 > bestDist2))
		{
			bestDist2 = dist2;
			bestEntity = entity;
		}
	}

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
