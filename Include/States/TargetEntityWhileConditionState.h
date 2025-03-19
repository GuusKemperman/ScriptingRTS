#pragma once
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "Core/EntityFilter.h"
#include "World/Registry.h"

namespace RTS
{
	class GameState;
	class GameEvaluateStep;

	template<typename Derived>
	class TargetEntityWhileConditionState
	{
	public:
		static void ForEach(const GameState& gameState, 
			std::span<const Derived> states, 
			const auto& func);

		static std::optional<Derived> EnterState(const GameState& gameState,
			entt::entity unit,
			EntityFilter target);

		bool operator==(const TargetEntityWhileConditionState&) const = default;
		bool operator!=(const TargetEntityWhileConditionState&) const = default;

		entt::entity mRequestedByUnit = entt::null;
		EntityFilter mTargetFilter{};
	};

	template <typename Derived>
	void TargetEntityWhileConditionState<Derived>::ForEach(const GameState& gameState,
		std::span<const Derived> states, const auto& func)
	{
		auto* entityStorage = gameState.GetWorld().GetRegistry().Storage<entt::entity>();

		if (entityStorage == nullptr)
		{
			return;
		}

		for (const TargetEntityWhileConditionState& state : states)
		{
			if (!entityStorage->contains(state.mRequestedByUnit))
			{
				continue;
			}

			entt::entity target = state.mTargetFilter(gameState, state.mRequestedByUnit);

			if (target == entt::null)
			{
				continue;
			}

			func(state.mRequestedByUnit, target);
		}
	}

	template <typename Derived>
	std::optional<Derived> TargetEntityWhileConditionState<Derived>::EnterState(const GameState& gameState,
		entt::entity unit, EntityFilter target)
	{
		if (target(gameState, unit) != entt::null)
		{
			return Derived{ unit, target };
		}
		return std::nullopt;
	}
}
