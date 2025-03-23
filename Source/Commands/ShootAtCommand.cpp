#include "Precomp.h"
#include "Commands/ShootAtCommand.h"

#include "Components/HealthComponent.h"
#include "Components/ProjectileComponent.h"
#include "Components/UnitSpawnPositionComponent.h"
#include "Components/UnitTypeTag.h"
#include "Components/WeaponComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameState.h"
#include "Utilities/Random.h"
#include "World/Registry.h"

void RTS::ShootAtCommand::Execute(GameState& state, std::span<const ShootAtCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto& transformStorage = reg.Storage<CE::TransformedDiskColliderComponent>();
	auto& projectileStorage = reg.Storage<ProjectileComponent>();
	auto& weaponStorage = reg.Storage<WeaponComponent>();
	auto& healthStorage = reg.Storage<HealthComponent>();
	auto& rngStorage = reg.Storage<CE::DefaultRandomEngine>();

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

		const float distance = glm::distance(startPos, endPos);
		const float accuracy = [&]()
			{
				float result = 0.0f;

				size_t i = weaponType.mRangeDistance.sRangeEnds.size();
				while (i --> 0)
				{
					if (distance > weaponType.mRangeDistance.sRangeEnds[i])
					{
						break;
					}

					result = weaponType.mRangeAccuracy.sAccuracies[i];
				}

				return result;
			}();

		const float randomNum = [&]
			{
				CE::DefaultRandomEngine& rng = rngStorage.get(command.mFiredBy);
				std::uniform_real_distribution distribution{ 0.0f, 1.0f };
				return distribution(rng);
			}();

		entt::entity projEntity = reg.Create();
		projectileStorage.emplace(projEntity,
			ProjectileComponent{
				.mFiredFromWeapon = weapon.mType,
				.mTargetEntity = command.mTargetEntity,
				.mSourcePosition = startPos,
				.mTargetPosition = endPos,
				.mNumStepsUntilImpact = static_cast<uint8>(numStepsUntilImpact),
				.mHasHit = healthStorage.contains(command.mTargetEntity) ? randomNum <= accuracy : false
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

		if (!proj.mHasHit 
			|| !transformStorage.contains(proj.mTargetEntity))
		{
			continue;
		}

		CE::TransformedDiskColliderComponent& targetDisk = transformStorage.get(proj.mTargetEntity);

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
			UnitSpawnPositionComponent& spawn = reg.Get<UnitSpawnPositionComponent>(proj.mTargetEntity);
			targetDisk.mCentre = spawn.mSpawnedAtPosition;

			const UnitType& type = GetUnitType(reg.Get<UnitType::Enum>(proj.mTargetEntity));
			health.mHealth =  type.mHealth;
		}
	}

	for (auto [entity, weapon] : weaponStorage.each())
	{
		--weapon.mNumStepsUntilCanFire;
	}

	// TODO find a better place for this logic
	for (auto [entity, health, type] : reg.View<HealthComponent, UnitType::Enum>().each())
	{
		const UnitType& unitType = GetUnitType(type);

		if (health.mHealth < unitType.mHealth)
		{
			health.mHealth = glm::min(unitType.mHealth, health.mHealth + unitType.mHealthRegeneration * Constants::sSimulationStepSize);
		}
	}
}
