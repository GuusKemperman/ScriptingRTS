#pragma once
#include "Meta/Fwd/MetaTypeIdFwd.h"

namespace RTS
{
	enum class TeamId : uint8 { Team1, Team2, Neutral };

	struct Team1Tag {};
	struct Team2Tag {};
	struct NeutralTag {};

	constexpr TeamId SwitchTeams(TeamId team)
	{
		return static_cast<TeamId>(!static_cast<bool>(team));
	}

	constexpr CE::TypeId GetTeamTagStorage(TeamId team)
	{
		switch (team)
		{
		case TeamId::Team1: return CE::MakeTypeId<Team1Tag>();
		case TeamId::Team2: return CE::MakeTypeId<Team2Tag>();
		case TeamId::Neutral: return CE::MakeTypeId<NeutralTag>();
		}
		ABORT;
		return {};
	}

	void SwitchTeams(
		entt::entity entity, 
		TeamId desiredTeam,
		entt::storage_for_t<Team1Tag>& team1,
		entt::storage_for_t<Team2Tag>& team2,
		entt::storage_for_t<NeutralTag>& neutral,
		entt::storage_for_t<TeamId>& teamId,
		bool isNewEntity);
}
