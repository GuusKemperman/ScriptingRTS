#pragma once
#include "FilterParams.h"
#include "Core/UnitTypes.h"

namespace RTS
{
	struct HealthComponent;

	struct HealthFilter
	{
		constexpr bool operator==(const HealthFilter&) const = default;
		constexpr bool operator!=(const HealthFilter&) const = default;

		enum Enum : uint8
		{
			From0To25 = 1 << 1,
			From25To50 = 1 << 2,
			From50To75 = 1 << 3,
			From75To100 = 1 << 4,
			Full = 1 << 5,
			Any = From0To25 | From25To50 | From50To75 | From75To100 | Full
		};
		Enum mEnum = Any;

		struct PerEntityCache
		{
			PerEntityCache(const MakeCacheParam& param);

			const entt::storage_for_t<HealthComponent>* mHealthStorage{};
			const entt::storage_for_t<UnitType::Enum>* mUnitTypeStorage{};
		};

		bool operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const;
	};
}

template <>
struct magic_enum::customize::enum_range<RTS::HealthFilter::Enum>
{
	static constexpr bool is_flags = true;
};