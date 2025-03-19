#include "Precomp.h"
#include "Core/Filters/TypeFilter.h"

#include "Core/EntityFilter.h"
#include "Core/GameState.h"
#include "Core/UnitTypes.h"
#include "World/Registry.h"

RTS::TypeFilter::PerEntityCache::PerEntityCache(const MakeCacheParam& param)
{
	const CE::Registry& reg = param.mState.GetWorld().GetRegistry();
	mObjectiveStorage = reg.Storage<ObjectiveComponent>();
	mUnitTypeStorage = reg.Storage<UnitType::Enum>();
}

bool RTS::TypeFilter::operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const
{
	return (param.mFilter.mType.mEnum & Unit 
				&& (perEntityCache.mUnitTypeStorage != nullptr && perEntityCache.mUnitTypeStorage->contains(param.mPotentialTarget)))
		|| (param.mFilter.mType.mEnum & Objective
			&& (perEntityCache.mObjectiveStorage != nullptr && perEntityCache.mObjectiveStorage->contains(param.mPotentialTarget)));
}
