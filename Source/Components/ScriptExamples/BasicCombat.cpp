#include "Precomp.h"
#include "Components/ScriptExamples/BasicCombatComponent.h"

#include "Core/ScriptingAPI.h"
#include "Utilities/Reflect/ReflectComponentType.h"

void RTS::Examples::BasicCombatComponent::OnUnitEvaluate(CE::World&, entt::entity)
{
	/**/   (RTSAPI::Condition({ .mTeam = TeamFilter::Myself, .mHealth = HealthFilter::Below50 })
	 &&		RTSAPI::Action(Action::FleeFrom, { .mTeam = TeamFilter::Enemy }))
	 ||		RTSAPI::Action(Action::ShootAt, { .mTeam = TeamFilter::Enemy, .mRange = RangeFilter::InMidRangeOrCloser })
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

