#include "Precomp.h"
#include "Components/RenderingComponent.h"

#include "Meta/MetaType.h"
#include "Utilities/Reflect/ReflectComponentType.h"

CE::MetaType RTS::RenderingComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RenderingComponent>{}, "RenderingComponent" };

	metaType.AddField(&RenderingComponent::mTimeStamp, "mTimeStamp");
	metaType.AddField(&RenderingComponent::mPlaySpeed, "mPlaySpeed");

	CE::ReflectComponentType<RenderingComponent>(metaType);
	return metaType;
}
