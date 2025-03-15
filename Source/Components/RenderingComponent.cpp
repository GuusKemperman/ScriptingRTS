#include "Precomp.h"
#include "Components/RenderingComponent.h"

#include "Components/SimulationComponent.h"
#include "Meta/MetaType.h"
#include "Systems/RenderingSystem.h"
#include "Utilities/Reflect/ReflectComponentType.h"

void RTS::RenderingComponent::OnSimulationStart(CE::World& viewportWorld, entt::entity owner)
{
	SimulationComponent& sim = viewportWorld.GetRegistry().GetAny<SimulationComponent>();

	sim.mOnStepCompletedCallback =
		[renderer = &viewportWorld.AddSystem<RenderingSystem>()](const GameSimulationStep& step)
		{
			renderer->RecordStep(step);
		};
	
}

CE::MetaType RTS::RenderingComponent::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RenderingComponent>{}, "RenderingComponent" };

	metaType.AddField(&RenderingComponent::mTimeStamp, "mTimeStamp");
	metaType.AddField(&RenderingComponent::mPlaySpeed, "mPlaySpeed");

	CE::BindEvent(metaType, sOnSimulationStart, &RenderingComponent::OnSimulationStart);

	CE::ReflectComponentType<RenderingComponent>(metaType);
	return metaType;
}
