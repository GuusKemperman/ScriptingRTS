#pragma once
#include "BasicDataTypes\CollisionRules.h"

namespace RTS
{
	enum class CollisionLayer : uint8
	{
		Team1Layer = CE::CollisionLayer::GameLayer0,
		Team2Layer = CE::CollisionLayer::GameLayer1,
	};

	constexpr CE::CollisionLayer ToCE(RTS::CollisionLayer layer) { return static_cast<CE::CollisionLayer>(layer); }
}