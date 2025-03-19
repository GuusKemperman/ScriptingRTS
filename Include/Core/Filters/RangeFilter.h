#pragma once
#include "FilterParams.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/WeaponTypes.h"

namespace RTS
{
	class GameState;

	struct RangeFilter
	{
		constexpr bool operator==(const RangeFilter&) const = default;
		constexpr bool operator!=(const RangeFilter&) const = default;

		enum Enum : uint8
		{
			ShortRange = 1 << 1,
			MidRange = 1 << 2,
			LongRange = 1 << 3,
			OutOfRange = 1 << 4,
			Any = 0xff
		};
		Enum mEnum = Any;

		struct PerEntityCache
		{
			PerEntityCache(const MakeCacheParam& param);

			static constexpr WeaponType::Enum sNullWeapon = static_cast<WeaponType::Enum>(std::numeric_limits<std::underlying_type_t<WeaponType::Enum>>::max());
			WeaponType::Enum mWeapon = sNullWeapon;

			glm::vec2 mQueryPosition{};
			const entt::storage_for_t<CE::TransformedDiskColliderComponent>* mDisksStorage{};
		};

		bool operator()(const CheckEntityParam& param, const PerEntityCache& perEntityCache) const;
	};
}
