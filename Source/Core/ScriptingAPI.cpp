#include "Precomp.h"
#include "Core/ScriptingAPI.h"

#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "States/MoveToEntityState.h"
#include "States/MoveToPositionState.h"
#include "States/ShootAtEntityState.h"
#include "World/Registry.h"

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

void RTS::RTSAPI::ShootAt(entt::entity target)
{
	EnterState<ShootAtEntityState>(target);
}

float RTS::RTSAPI::GetDistance(entt::entity entity)
{
	CheckTarget();

	auto* diskStorage = sTarget.sCurrentState->GetWorld().GetRegistry().Storage<CE::TransformedDiskColliderComponent>();

	if (diskStorage == nullptr
		|| !diskStorage->contains(entity))
	{
		return std::numeric_limits<float>::infinity();
	}

	return glm::distance(diskStorage->get(sTarget.sCurrentUnit).mCentre, diskStorage->get(entity).mCentre);
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

	metaType.AddFunc(&RTSAPI::ShootAt,
		"ShootAt",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::FindEntity,
		"FindEntity",
		"Filter").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, true);

	metaType.AddFunc(&RTSAPI::GetDistance,
		"GetDistance").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, true);

	return metaType;
}
