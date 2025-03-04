#pragma once
#include "Meta/Fwd/MetaReflectFwd.h"

namespace RTS
{
	struct RenderingComponent
	{
		float mTimeStamp{};
		float mPlaySpeed = 1.0f;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(RenderingComponent);
	};
}
