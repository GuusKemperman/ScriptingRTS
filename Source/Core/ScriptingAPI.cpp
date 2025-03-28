#include "Precomp.h"
#include "Core/ScriptingAPI.h"

#include "Core/GameEvaluateStep.h"
#include "Meta/ReflectedTypes/ReflectEnums.h"
#include "States/FleeFromEntityState.h"
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

bool RTS::RTSAPI::Action(RTS::Action action, EntityFilter target)
{
	switch (action)
	{
	case Action::MoveTo:
		return EnterState<MoveToEntityState>(target);
	case Action::FleeFrom:
		return EnterState<FleeFromEntityState>(target);
	case Action::ShootAt:
		return EnterState<ShootAtEntityState>(target);
	}
	ABORT;
	return false;
}

bool RTS::RTSAPI::Condition(EntityFilter target)
{
	CheckTarget();
	return target(*sTarget.sCurrentState, sTarget.sCurrentUnit) != entt::null;
}

CE::MetaType RTS::RTSAPI::Reflect()
{
	CE::MetaType metaType{ CE::MetaType::T<RTSAPI>{}, "RTS" };
	metaType.GetProperties().Add(CE::Props::sIsScriptableTag);

	metaType.AddFunc(&RTSAPI::Action,
		"Action",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, false);

	metaType.AddFunc(&RTSAPI::Condition,
		"Condition",
		"Target").GetProperties().Add(CE::Props::sIsScriptableTag).Set(CE::Props::sIsScriptPure, true);

	return metaType;
}

CE::MetaType Reflector<RTS::Action>::Reflect()
{
	return CE::ReflectEnumType<RTS::Action>(true);
}
