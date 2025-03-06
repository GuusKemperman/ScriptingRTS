#pragma once

namespace RTS
{
	enum class TeamId : bool { Team1, Team2 };

	template<TeamId>
	struct TeamTag {};

	using Team1Tag = TeamTag<TeamId::Team1>;
	using Team2Tag = TeamTag<TeamId::Team2>;
}