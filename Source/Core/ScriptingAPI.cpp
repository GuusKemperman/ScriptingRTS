#include "Precomp.h"
#include "Core/ScriptingAPI.h"

#include "Core/GameState.h"
#include "States/MoveToEntityState.h"
#include "States/MoveToPositionState.h"

namespace
{
	thread_local RTS::Internal::OnUnitEvaluateTarget sTarget{};

	void CheckTarget()
	{
		ASSERT_LOG(sTarget.sNextStep != nullptr
			&& sTarget.sCurrentState != nullptr, "Tried accessing simulation functions from a world that was not part of the simulation.");
	}

	template<typename State, typename... Args>
	void EnterState(Args&&... args)
	{
		CheckTarget();

		State::EnterState(*sTarget.sCurrentState, 
			*sTarget.sNextStep,
			sTarget.sCurrentUnit,
			std::forward<Args>(args)...);
	}
}

void RTS::Internal::SetOnUnitEvaluateTargetForCurrentThread(OnUnitEvaluateTarget target)
{
	sTarget = target;
}

void RTS::RTSAPI::MoveToEntity(entt::entity target)
{
	EnterState<MoveToEntityState>(target);
}

void RTS::RTSAPI::MoveToPosition(glm::vec2 target)
{
	EnterState<MoveToPositionState>(target);
}

entt::entity RTS::RTSAPI::FindEntity(UnitFilter filter)
{
	CheckTarget();
	filter.mRequestedByUnit = sTarget.sCurrentUnit;
	return filter(sTarget.sCurrentState->GetWorld());
}

CE::MetaType RTS::RTSAPI::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RTSAPI>{}, "RTS" };
	metaType.GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::MoveToEntity,
		"MoveToEntity",
		"TargetEntity").GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::MoveToPosition,
		"MoveToPosition",
		"TargetPosition").GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::FindEntity,
		"FindEntity",
		"Filter").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, true);

	return metaType;
}
