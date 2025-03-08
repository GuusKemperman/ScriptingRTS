#include "Precomp.h"
#include "States/ShootAtEntityState.h"

#include "Commands/CommandBuffer.h"
#include "Commands/ShootAtCommand.h"
#include "Components/WeaponComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameEvaluateStep.h"
#include "Core/GameSimulationStep.h"
#include "Core/GameState.h"
#include "World/Registry.h"
#include "Utilities/Math.h"

void RTS::ShootAtEntityState::Execute(const GameState& gameState, 
	GameSimulationStep& nextStep,
	std::span<const ShootAtEntityState> commands)
{
	const CE::Registry& reg = gameState.GetWorld().GetRegistry();

	auto* diskStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto* weaponStorage = reg.Storage<WeaponComponent>();

	if (diskStorage == nullptr
		|| weaponStorage == nullptr)
	{
		return;
	}

	CommandBuffer<ShootAtCommand>& shootAtBuffer = nextStep.GetBuffer<ShootAtCommand>();

	for (const ShootAtEntityState& shootState : commands)
	{
		if (!weaponStorage->contains(shootState.mUnit)
			|| !diskStorage->contains(shootState.mTargetEntity))
		{
			continue;
		}

		const WeaponComponent& weapon = weaponStorage->get(shootState.mUnit);

		if (weapon.mNumStepsUntilCanFire > 0)
		{
			continue;
		}

		ASSERT(diskStorage->contains(shootState.mUnit));

		const CE::TransformedDisk& unitDisk = diskStorage->get(shootState.mUnit);
		const CE::TransformedDisk& targetDisk = diskStorage->get(shootState.mTargetEntity);

		const float range2 = CE::Math::sqr(GetWeaponProperty<&WeaponType::mMaxRange>(weapon.mType));
		const float dist2 = glm::distance2(unitDisk.mCentre, targetDisk.mCentre);

		if (dist2 > range2)
		{
			continue;
		}

		shootAtBuffer.AddCommand(ShootAtCommand
			{
				.mFiredBy = shootState.mUnit,
				.mTargetEntity = shootState.mTargetEntity
			});
	}
}

void RTS::ShootAtEntityState::EnterState(const GameState&,
	GameEvaluateStep& nextStep,
	entt::entity unit,
	entt::entity targetEntity)
{
	nextStep.AddCommand(ShootAtEntityState{ unit, targetEntity });
}
