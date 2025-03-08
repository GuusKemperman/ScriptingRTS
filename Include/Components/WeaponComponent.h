#pragma once
#include "Core/WeaponTypes.h"

namespace RTS
{
	struct WeaponComponent
	{
		WeaponType::Enum mType{};
		int32 mNumStepsUntilCanFire{};
	};
}
