#include "Precomp.h"
#include "Components/ScriptExamples/BasicCaptureComponent.h"

#include "Core/ScriptingAPI.h"
#include "Utilities/Reflect/ReflectComponentType.h"

using namespace magic_enum::bitwise_operators;

void RTS::Examples::BasicCaptureComponent::OnUnitEvaluate(CE::World&, entt::entity)
{
	RTSAPI::Action(Action::MoveTo, { .mType = TypeFilter::Objective, .mTeam = TeamFilter::Enemy | TeamFilter::Neutral });
}

CE::MetaType RTS::Examples::BasicCaptureComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<BasicCaptureComponent>{}, "BasicCaptureComponent" };

	CE::BindEvent(metaType, sOnUnitEvaluate, &BasicCaptureComponent::OnUnitEvaluate);

	CE::ReflectComponentType<BasicCaptureComponent>(metaType);

	return metaType;
}

