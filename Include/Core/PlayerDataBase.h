#pragma once
#include <shared_mutex>

#include "GameState.h"
#include "Scripting/CompiledProgram.h"

namespace RTS
{
	class PlayerDataBase
	{
	public:
		struct Player
		{
			CompiledProgram mProgram{};
			std::string mName{};

			std::shared_mutex mEloMutex{};
			float mElo = 1500.0f;
			size_t mGamesPlayed{};
		};

		void AddPlayer(CompiledProgram program, std::string name);

		size_t GetNumberOfPlayers() const;

		std::pair<std::shared_lock<std::shared_mutex>, const std::vector<std::shared_ptr<Player>>&> OpenForRead() const;

		std::pair<std::unique_lock<std::shared_mutex>, std::vector<std::shared_ptr<Player>>&> OpenForWrite();

		std::shared_ptr<Player> FindMatch(const Player& forPlayer);

	private:
		static GameState Battle(const Player& team1, const Player& team2);

		static void UpdateELO(Player& team1, Player& team2, const GameState& result);

		mutable std::shared_mutex mPlayersMutex{};
		std::vector<std::shared_ptr<Player>> mPlayers{};
	};
}
