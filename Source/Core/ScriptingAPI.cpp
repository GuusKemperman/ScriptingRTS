#include "Precomp.h"
#include "Core/ScriptingAPI.h"

#include "Core/GameEvaluateStep.h"
#include "States/MoveToEntityState.h"
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

		sTarget.sNextStep->AddCommand(State{ sTarget.sCurrentUnit, std::forward<Args>(args)... });
	}
}

void RTS::Internal::SetOnUnitEvaluateTargetForCurrentThread(OnUnitEvaluateTarget target)
{
	sTarget = target;
}

void RTS::RTSAPI::MoveToEntity(UnitFilter target)
{
	EnterState<MoveToEntityState>(target);
}

void RTS::RTSAPI::ShootAt(UnitFilter target)
{
	EnterState<ShootAtEntityState>(target);
}

CE::MetaType RTS::RTSAPI::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RTSAPI>{}, "RTS" };
	metaType.GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::MoveToEntity,
		"MoveToEntity",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::ShootAt,
		"ShootAt",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag);

	return metaType;
}
