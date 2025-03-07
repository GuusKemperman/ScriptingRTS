#include "Precomp.h"
#include "Core/ScriptingAPI.h"

#include "Components/SimulationComponent.h"

namespace
{
	std::tuple<RTS::GameEvaluateStep&, const RTS::GameState&> GetStepAndState(CE::World& world)
	{
		RTS::SimulationComponent* simulationComponent = RTS::SimulationComponent::TryGetOwningSimulationComponent(world);

		ASSERT_LOG(simulationComponent != nullptr, "Tried accessing simulation functions from a world that was not part of the simulation.");

		RTS::GameEvaluateStep& step = simulationComponent->GetNextEvaluateStep();
		const RTS::GameState& state = simulationComponent->GetGameState();
		return { step, state };
	}

	template<typename State, typename... Args>
	void EnterState(CE::World& world, Args&&... args)
	{
		auto [step, state] = GetStepAndState(world);
		State::EnterState(state, step, std::forward<Args>(args)...);
	}
}

void RTS::RTS::MoveToEntity(CE::World& world, entt::entity unit, entt::entity target)
{
	EnterState<MoveToEntityState>(world, unit, target);
}

void RTS::RTS::MoveToPosition(CE::World& world, entt::entity unit, glm::vec2 target)
{
	EnterState<MoveToPositionState>(world, unit, target);
}

entt::entity RTS::RTS::FindEntity(const CE::World& world, entt::entity unit, UnitFilter filter)
{
	filter.mRequestedByUnit = unit;
	return filter(world);
}

CE::MetaType RTS::RTS::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RTS>{}, "RTS" };
	metaType.GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTS::MoveToEntity, 
		"MoveToEntity",
		"World",
		"UnitToMove",
		"TargetEntity").GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTS::MoveToPosition,
		"MoveToPosition",
		"World",
		"UnitToMove",
		"TargetPosition").GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTS::FindEntity,
		"FindEntity",
		"World",
		"FromUnit",
		"Filter").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, true);

	return metaType;
}
