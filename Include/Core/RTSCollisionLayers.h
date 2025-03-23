#pragma once
#include "EntityTypes.h"
#include "BasicDataTypes\CollisionRules.h"

namespace RTS
{
	enum class CollisionLayer : uint8
	{
		Team1Layer = CE::CollisionLayer::GameLayer0,
		Team2Layer = CE::CollisionLayer::GameLayer1,
		Objectives = CE::CollisionLayer::GameLayer2,
	};

	constexpr CE::CollisionLayer ToCE(RTS::CollisionLayer layer) { return static_cast<CE::CollisionLayer>(layer); }

	constexpr std::optional<CollisionLayer> GetCollisionLayer(TeamId team)
	{
		switch (team)
		{
		case TeamId::Team1: return CollisionLayer::Team1Layer;
		case TeamId::Team2: return CollisionLayer::Team2Layer;
		case TeamId::Neutral: return std::nullopt;
		}
		return std::nullopt;
	}
}