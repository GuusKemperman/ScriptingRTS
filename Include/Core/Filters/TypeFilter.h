#pragma once
#include "FilterParams.h"
#include "Core/UnitTypes.h"
#include "Components/ObjectiveComponent.h"

namespace RTS
{
	class GameState;

	struct TypeFilter
	{
		constexpr bool operator==(const TypeFilter&) const = default;
		constexpr bool operator!=(const TypeFilter&) const = default;

		enum Enum : uint8
		{
			Unit = 1 << 1,
			Objective = 1 << 2,
			Any = 0xff
		};
		Enum mEnum = Any;

		struct PerEntityCache
		{
			PerEntityCache(const MakeCacheParam& param);

			const entt::storage_for_t<ObjectiveComponent>* mObjectiveStorage{};
			const entt::storage_for_t<UnitType::Enum>* mUnitTypeStorage{};
		};

		bool operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const;
	};
}
