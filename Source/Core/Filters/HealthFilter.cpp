#include "Precomp.h"
#include "Core/Filters/HealthFilter.h"

#include "Components/HealthComponent.h"
#include "Core/EntityFilter.h"
#include "Core/GameState.h"
#include "Core/UnitTypes.h"
#include "World/Registry.h"

RTS::HealthFilter::PerEntityCache::PerEntityCache(const MakeCacheParam& param)
{
	const CE::Registry& reg = param.mState.GetWorld().GetRegistry();
	mHealthStorage = reg.Storage<HealthComponent>();
	mUnitTypeStorage = reg.Storage<UnitType::Enum>();
}

bool RTS::HealthFilter::operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const
{
	if (perEntityCache.mHealthStorage == nullptr
		|| perEntityCache.mUnitTypeStorage == nullptr
		|| !perEntityCache.mUnitTypeStorage->contains(param.mPotentialTarget)
		|| !perEntityCache.mHealthStorage->contains(param.mPotentialTarget))
	{
		return false;
	}

	const UnitType unitType = GetUnitType(perEntityCache.mUnitTypeStorage->get(param.mPotentialTarget));
	const float unitHealth = perEntityCache.mHealthStorage->get(param.mPotentialTarget).mHealth;
	const float healthPercentage = unitHealth / unitType.mHealth;

	return (healthPercentage <= 0.25f && param.mFilter.mHealth.mEnum & From0To25)
		|| (healthPercentage >= 0.25f && healthPercentage <= 0.50f && param.mFilter.mHealth.mEnum & From25To50)
		|| (healthPercentage >= 0.50f && healthPercentage <= 0.75f && param.mFilter.mHealth.mEnum & From50To75)
		|| (healthPercentage >= 0.75f && param.mFilter.mHealth.mEnum & From75To100)
		|| (healthPercentage >= 1.0f - std::numeric_limits<float>::epsilon() && param.mFilter.mHealth.mEnum & Full);
}
