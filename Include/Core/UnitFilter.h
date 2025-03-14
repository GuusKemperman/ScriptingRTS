#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace RTS
{
	enum class TeamFilter : uint8 { Any, Friendly, Enemy };
	enum class SortPriority : uint8 { Nearest, Farthest };
	enum class RangeFilter : uint8 {
		Any,
		InShortRange,
		WithinShortRange,
		OutShortRange,
		InMidRange,
		WithinMidRange,
		OutMidRange,
		InLongRange,
		WithinLongRange,
		OutLongRange,
	};

	struct UnitFilter
	{
#ifdef EDITOR
		void DisplayWidget(const std::string& name);
#endif

		constexpr auto operator<=>(const UnitFilter&) const = default;

		entt::entity operator()(const CE::World& world, entt::entity requestedByUnit) const;

		TeamFilter mTeam{};
		SortPriority mSortByDistance{};
		RangeFilter mRange{};

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(UnitFilter);
	};
}

template<>
struct Reflector<RTS::TeamFilter>
{
	static CE::MetaType Reflect();
};

template<>
struct Reflector<RTS::SortPriority>
{
	static CE::MetaType Reflect();
};

template<>
struct Reflector<RTS::RangeFilter>
{
	static CE::MetaType Reflect();
};

#ifdef EDITOR
IMGUI_AUTO_DEFINE_INLINE(template<>, RTS::UnitFilter, var.DisplayWidget(name);)
#endif // EDITOR

namespace cereal
{
	template<typename Archive>
	void serialize(Archive& ar, RTS::UnitFilter& value, [[maybe_unused]] uint32 version)
	{
		ar(value.mSortByDistance);
		ar(value.mTeam);

		if (version >= 1)
		{
			ar(value.mRange);
		}
	}
}

CEREAL_CLASS_VERSION(RTS::UnitFilter, 1);

