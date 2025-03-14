#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace RTS
{
	enum class TeamFilter : uint8 { Any, Friendly, Enemy, Myself };
	enum class SortPriority : uint8 { Nearest, Farthest };

	enum class RangeFilter : uint8 {
		Any,
		InShortRange,
		InShortRangeOrCloser,
		OutShortRange,
		InMidRange,
		InMidRangeOrCloser,
		OutMidRange,
		InLongRange,
		InLongRangeOrCloser,
		OutLongRange,
	};

	enum class HealthFilter : uint8
	{
		Any,
		Below25,
		Above25,
		Below50,
		Above50,
		Below75,
		Above75,
		Below100,
		Full,
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
		HealthFilter mHealth{};

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

template<>
struct Reflector<RTS::HealthFilter>
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

		if (version >= 2)
		{
			ar(value.mHealth);
		}
	}
}

CEREAL_CLASS_VERSION(RTS::UnitFilter, 2);

