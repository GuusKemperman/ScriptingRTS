#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace CE
{
	class World;
}

namespace RTS
{
	enum class TeamFilter : uint8 { Any, Friendly, Enemy };
	enum class DistanceFilter : uint8 { Nearest, Farthest };

	struct UnitFilter
	{
#ifdef EDITOR
		void DisplayWidget(const std::string& name);
#endif

		constexpr auto operator<=>(const UnitFilter&) const = default;

		entt::entity operator()(const CE::World& world, entt::entity requestedByUnit) const;

		TeamFilter mTeam{};
		DistanceFilter mSortByDistance{};

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
struct Reflector<RTS::DistanceFilter>
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
	}
}

