#pragma once
#include <bitset>

#include "FilterParams.h"
#include "Components/TeamTag.h"
#include "Core/UnitTypes.h"

namespace RTS
{
	struct TeamFilter
	{
		constexpr bool operator==(const TeamFilter&) const = default;
		constexpr bool operator!=(const TeamFilter&) const = default;

		enum Enum : uint8
		{
			Friendly = 1 << 1,
			Neutral = 1 << 2,
			Enemy = 1 << 3,
			Myself = 1 << 4,
			Any = 0xff

		};
		Enum mEnum{};

		struct PerEntityCache
		{
			PerEntityCache(const MakeCacheParam& param);

			const entt::storage_for_t<TeamId>* mTeamIdStorage{};
			std::bitset<magic_enum::enum_count<TeamId>()> isTeamAllowed{};
		};

		bool operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const;
	};
}
