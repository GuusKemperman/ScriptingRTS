#include "Precomp.h"
#include "Commands/ShootAtCommand.h"

#include "Components/HealthComponent.h"
#include "Components/ProjectileComponent.h"
#include "Components/UnitTypeTag.h"
#include "Components/WeaponComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::ShootAtCommand::Execute(GameState& state, std::span<const ShootAtCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	const auto& transformStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto& projectileStorage = reg.Storage<ProjectileComponent>();
	auto& weaponStorage = reg.Storage<WeaponComponent>();
	auto& healthStorage = reg.Storage<HealthComponent>();

	for (const ShootAtCommand& command : commands)
	{
		WeaponComponent& weapon = weaponStorage.get(command.mFiredBy);
		const WeaponType weaponType = GetWeaponType(weapon.mType);

		// + 1 cus we lower it by one in a second.
		weapon.mNumStepsUntilCanFire = weaponType.mNumStepsBetweenFiring + 1;

		const glm::vec2 startPos = transformStorage.get(command.mFiredBy).mCentre;
		const glm::vec2 endPos = transformStorage.get(command.mTargetEntity).mCentre;
		
		// + 1 cus we lower it by one in a second.
		const int numStepsUntilImpact = weaponType.mNumStepsUntilImpact + 1;

		entt::entity projEntity = reg.Create();
		projectileStorage.emplace(projEntity,
			ProjectileComponent{
				.mFiredFromWeapon = weapon.mType,
				.mTargetEntity = command.mTargetEntity,
				.mSourcePosition = startPos,
				.mTargetPosition = endPos,
				.mNumStepsUntilImpact = numStepsUntilImpact
			});
	}

	for (auto [entity, proj] : projectileStorage.each())
	{
		if (--proj.mNumStepsUntilImpact > 0)
		{
			continue;
		}

		if (proj.mNumStepsUntilImpact <= -1)
		{
			reg.Destroy(entity, false);
			continue;
		}

		if (!transformStorage.contains(proj.mTargetEntity))
		{
			continue;
		}

		const CE::TransformedDiskColliderComponent& targetDisk = transformStorage.get(proj.mTargetEntity);

		if (!CE::AreOverlapping(targetDisk, proj.mTargetPosition))
		{
			continue;
		}

		HealthComponent& health = healthStorage.get(proj.mTargetEntity);

		if (health.mHealth <= 0.0f)
		{
			continue;
		}

		const WeaponType weaponType = GetWeaponType(proj.mFiredFromWeapon);
		health.mHealth -= weaponType.mDamage;

		if (health.mHealth <= 0.0f)
		{
			reg.Destroy(proj.mTargetEntity, false);
		}
	}

	for (auto [entity, weapon] : weaponStorage.each())
	{
		--weapon.mNumStepsUntilCanFire;
	}
}
