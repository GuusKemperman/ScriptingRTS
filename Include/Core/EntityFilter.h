#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"
#include "Core/Filters/HealthFilter.h"
#include "Filters/RangeFilter.h"
#include "Filters/TeamFilter.h"
#include "Filters/TypeFilter.h"

namespace CE
{
	class World;
}

namespace RTS
{
	enum class SortPriority : uint8
	{
		Nearest,
		Farthest
	};

	struct EntityFilter
	{
#ifdef EDITOR
		void DisplayWidget(const std::string& name);
#endif

		constexpr bool operator==(const EntityFilter&) const = default;
		constexpr bool operator!=(const EntityFilter&) const = default;

		entt::entity operator()(const GameState& state, entt::entity requestedByUnit) const;

		SortPriority mSortByDistance{};
		TypeFilter mType{};
		TeamFilter mTeam{};
		RangeFilter mRange{};
		HealthFilter mHealth{};

		constexpr void ForEachFilter(const auto& func) const;
		constexpr void ForEachFilter(const auto& func);

	private:
		static constexpr void ForEachFilter(auto& self, const auto& func);

		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(EntityFilter);
	};

	constexpr void EntityFilter::ForEachFilter(const auto& func) const
	{
		ForEachFilter(*this, func);
	}

	constexpr void EntityFilter::ForEachFilter(const auto& func)
	{
		ForEachFilter(*this, func);
	}

	constexpr void EntityFilter::ForEachFilter(auto& self, const auto& func)
	{
		func(self.mType);
		func(self.mTeam);
		func(self.mRange);
		func(self.mHealth);
	}
}

template<>
struct Reflector<RTS::SortPriority>
{
	static CE::MetaType Reflect();
};

#ifdef EDITOR
IMGUI_AUTO_DEFINE_INLINE(template<>, RTS::EntityFilter, var.DisplayWidget(name);)
#endif // EDITOR

namespace cereal
{
	template<typename Archive>
	void serialize([[maybe_unused]] Archive& ar, [[maybe_unused]] RTS::EntityFilter& value, [[maybe_unused]] uint32 version)
	{
		ar(value.mSortByDistance);

		value.ForEachFilter([&](auto& filter)
			{
				ar(filter.mEnum);
			});
	}
}

CEREAL_CLASS_VERSION(RTS::EntityFilter, 0);

