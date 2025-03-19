#include "Precomp.h"
#include "Core/Filters/RangeFilter.h"

#include "Components/WeaponComponent.h"
#include "Core/EntityFilter.h"
#include "Core/GameState.h"
#include "World/Registry.h"

RTS::RangeFilter::PerEntityCache::PerEntityCache(const MakeCacheParam& param)
{
	const CE::Registry& reg = param.mState.GetWorld().GetRegistry();

	const WeaponComponent* weapon = reg.TryGet<WeaponComponent>(param.mRequestedBy);

	if (weapon == nullptr)
	{
		return;
	}

	mWeapon = weapon->mType;

	mDisksStorage = reg.Storage<CE::TransformedDiskColliderComponent>();

	if (mDisksStorage == nullptr)
	{
		return;
	}

	ASSERT(mDisksStorage->contains(param.mRequestedBy));
	mQueryPosition = mDisksStorage->get(param.mRequestedBy).mCentre;
}

bool RTS::RangeFilter::operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const
{
	if (perEntityCache.mWeapon == PerEntityCache::sNullWeapon
		|| perEntityCache.mDisksStorage == nullptr
		|| !perEntityCache.mDisksStorage->contains(param.mPotentialTarget))
	{
		return false;
	}

	const WeaponType& weaponType = GetWeaponType(perEntityCache.mWeapon);
	const auto& range = weaponType.mRangeDistance.mNamedRanges;
	const float dist = perEntityCache.mDisksStorage->get(param.mPotentialTarget).SignedDistance(perEntityCache.mQueryPosition);

	return (dist <= range.mShortRangeEnd && param.mFilter.mRange.mEnum & ShortRange)
		|| (dist >= range.mShortRangeEnd && dist <= range.mMidRangeEnd && param.mFilter.mRange.mEnum & MidRange)
		|| (dist >= range.mMidRangeEnd && dist <= range.mLongRangeEnd && param.mFilter.mRange.mEnum & LongRange)
		|| (dist >= range.mLongRangeEnd && param.mFilter.mRange.mEnum & OutOfRange);
}
