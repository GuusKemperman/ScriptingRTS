#include "Precomp.h"
#include "States/ShootAtEntityState.h"

#include "Commands/CommandBuffer.h"
#include "Commands/ShootAtCommand.h"
#include "Components/WeaponComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
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

	ForEach(gameState, commands,
		[&](entt::entity unit, entt::entity target)
		{
			if (!weaponStorage->contains(unit)
				|| !diskStorage->contains(target))
			{
				return;
			}

			const WeaponComponent& weapon = weaponStorage->get(unit);

			if (weapon.mNumStepsUntilCanFire > 0)
			{
				return;
			}

			ASSERT(diskStorage->contains(unit));

			const CE::TransformedDisk& unitDisk = diskStorage->get(unit);
			const CE::TransformedDisk& targetDisk = diskStorage->get(target);

			const WeaponType weaponType = GetWeaponType(weapon.mType);
			const float range2 = CE::Math::sqr(weaponType.mRangeDistance.mNamedRanges.mLongRangeEnd);
			const float dist2 = glm::distance2(unitDisk.mCentre, targetDisk.mCentre);

			if (dist2 > range2)
			{
				return;
			}

			shootAtBuffer.AddCommand(ShootAtCommand
				{
					.mFiredBy = unit,
					.mTargetEntity = target
				});
		});
}
