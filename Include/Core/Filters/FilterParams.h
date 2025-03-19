#pragma once

namespace RTS
{
	class GameState;
	struct EntityFilter;

	struct MakeCacheParam
	{
		const GameState& mState;
		const EntityFilter& mFilter;
		entt::entity mRequestedBy;
	};

	struct CheckEntityParam
	{
		const GameState& mState;
		const EntityFilter& mFilter;
		entt::entity mRequestedBy;
		entt::entity mPotentialTarget{};
	};
}
