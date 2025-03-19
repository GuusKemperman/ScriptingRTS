#include "Precomp.h"
#include "Components/ScriptExamples/BasicCombatComponent.h"

#include "Core/ScriptingAPI.h"
#include "Utilities/Reflect/ReflectComponentType.h"

using namespace magic_enum::bitwise_operators;

void RTS::Examples::BasicCombatComponent::OnUnitEvaluate(CE::World&, entt::entity)
{
	/**/   (RTSAPI::Condition({ .mTeam = TeamFilter::Myself, .mHealth = HealthFilter::From0To25 | HealthFilter::From25To50 })
	 &&		RTSAPI::Action(Action::FleeFrom, { .mTeam = TeamFilter::Enemy }))
	 ||		RTSAPI::Action(Action::ShootAt, { .mTeam = TeamFilter::Enemy, .mRange = RangeFilter::ShortRange | RangeFilter::MidRange })
	 ||		RTSAPI::Action(Action::MoveTo, { .mTeam = TeamFilter::Enemy });
	/**/
}

CE::MetaType RTS::Examples::BasicCombatComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<BasicCombatComponent>{}, "BasicCombatComponent" };

	CE::BindEvent(metaType, sOnUnitEvaluate, &BasicCombatComponent::OnUnitEvaluate);

	CE::ReflectComponentType<BasicCombatComponent>(metaType);

	return metaType;
}

