#pragma once
#include "CommandBuffer.h"
#include "Components/UnitTypeTag.h"
#include "Components/Physics2D/DiskColliderComponent.h"

namespace RTS
{
	class GameState;

	class MoveToCommand
	{
	public:
		static void Execute(GameState& state, std::span<const MoveToCommand> commands);

		static void AddMoveTowardsPositionCommand(
			const entt::storage_for_t<CE::TransformedDiskColliderComponent>& transformStorage,
			const entt::storage_for_t<UnitType::Enum>& unitStorage,
			CommandBuffer<MoveToCommand>& moveCommandBuffer,
			entt::entity unit,
			glm::vec2 targetPosition);

		bool operator==(const MoveToCommand&) const = default;
		bool operator!=(const MoveToCommand&) const = default;

		entt::entity mUnit = entt::null;
		glm::vec2 mPosition{};
	};
}
