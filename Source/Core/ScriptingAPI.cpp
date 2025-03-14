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
	bool EnterState(Args&&... args)
	{
		CheckTarget();

		std::optional<State> state = State::EnterState(
			*sTarget.sCurrentState,
			sTarget.sCurrentUnit,
			std::forward<Args>(args)...);

		if (state.has_value())
		{
			sTarget.sNextStep->AddCommand(std::move(*state));
			return true;
		}
		return false;
	}
}

void RTS::Internal::SetOnUnitEvaluateTargetForCurrentThread(OnUnitEvaluateTarget target)
{
	sTarget = target;
}

bool RTS::RTSAPI::MoveToEntity(UnitFilter target)
{
	return EnterState<MoveToEntityState>(target);
}

bool RTS::RTSAPI::ShootAt(UnitFilter target)
{
	return EnterState<ShootAtEntityState>(target);
}

CE::MetaType RTS::RTSAPI::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RTSAPI>{}, "RTS" };
	metaType.GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::MoveToEntity,
		"MoveToEntity",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc(&RTSAPI::ShootAt,
		"ShootAt",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, false);

	return metaType;
}
