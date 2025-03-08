#pragma once
#include "Meta/Fwd/MetaTypeIdFwd.h"

namespace RTS
{
	enum class TeamId : bool { Team1, Team2 };

	struct Team1Tag {};
	struct Team2Tag {};

	constexpr TeamId SwitchTeams(TeamId team)
	{
		return static_cast<TeamId>(!static_cast<bool>(team));
	}

	constexpr CE::TypeId GetTeamTagStorage(TeamId team)
	{
		if (team == TeamId::Team1)
		{
			return CE::MakeTypeId<Team1Tag>();
		}
		return CE::MakeTypeId<Team2Tag>();
	}

}
