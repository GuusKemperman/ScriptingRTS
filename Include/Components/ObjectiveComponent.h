#pragma once

namespace RTS
{
	struct ObjectiveComponent
	{
		// -1.0f for team1, 0.0f for neutral, 1.0f for team2
		float mAmountCaptured{};

		static constexpr float sCaptureDuration = 10.0f;

		static constexpr float sPassiveAmountCapturedLossPerSecond = 1.0f;
	};
}
