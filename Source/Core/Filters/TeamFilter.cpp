#include "Precomp.h"
#include "Core/Filters/TeamFilter.h"

#include "Core/EntityFilter.h"
#include "Core/GameState.h"
#include "Core/RTSCollisionLayers.h"
#include "World/Registry.h"

RTS::TeamFilter::PerEntityCache::PerEntityCache(const MakeCacheParam& param)
{
	const CE::Registry& reg = param.mState.GetWorld().GetRegistry();
	mTeamIdStorage = reg.Storage<TeamId>();

	if (mTeamIdStorage == nullptr
		|| !mTeamIdStorage->contains(param.mRequestedBy))
	{
		return;
	}

	const TeamId myTeamId = mTeamIdStorage->get(param.mRequestedBy);

	auto addTeamId = [&](TeamId targetTeamId)
		{
			isTeamAllowed[static_cast<int>(targetTeamId)] = true;
			
			std::optional layer = GetCollisionLayer(targetTeamId);
		
			if (param.mFilter.mType.mEnum & TypeFilter::Unit
				&& layer.has_value())
			{
				param.mCollisionRules.SetResponse(ToCE(*layer), CE::CollisionResponse::Overlap);
			}
		};

	if (param.mFilter.mTeam.mEnum & Friendly)
	{
		addTeamId(myTeamId);
	}
	if (param.mFilter.mTeam.mEnum & Neutral)
	{
		addTeamId(TeamId::Neutral);
	}
	if (param.mFilter.mTeam.mEnum & Enemy)
	{
		addTeamId(SwitchTeams(myTeamId));
	}
}

bool RTS::TeamFilter::operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const
{
	if ((param.mFilter.mTeam.mEnum & Myself) && param.mPotentialTarget == param.mRequestedBy)
	{
		return true;
	}

	if (!perEntityCache.mTeamIdStorage->contains(param.mPotentialTarget))
	{
		return false;
	}

	TeamId targetTeam = perEntityCache.mTeamIdStorage->get(param.mPotentialTarget);

	return perEntityCache.isTeamAllowed[static_cast<int>(targetTeam)];
}
