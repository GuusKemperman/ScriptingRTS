#include "Precomp.h"
#include "Systems/RenderingSystem.h"

#include "Components/CameraComponent.h"
#include "Components/ProjectileComponent.h"
#include "Components/RenderingComponent.h"
#include "Components/SimulationComponent.h"
#include "Components/TransformComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/AssetManager.h"
#include "Core/Input.h"
#include "Core/Renderer.h"
#include "Utilities/DrawDebugHelpers.h"
#include "Utilities/Math.h"
#include "Utilities/Random.h"
#include "World/Registry.h"
#include "World/World.h"


RTS::RenderingSystem::RenderingSystem()
{
	CE::AssetManager& assetManager = CE::AssetManager::Get();

	mMat = assetManager.TryGetAsset<CE::Material>("MT_White");
	mMesh = assetManager.TryGetAsset<CE::StaticMesh>("SM_Plane");
	ASSERT(mMat != nullptr
		&& mMesh != nullptr);
}

void RTS::RenderingSystem::RecordStep(const GameSimulationStep& step)
{
	std::lock_guard guard{ mRenderingQueueMutex };
	mRenderingQueue.emplace_back(step);
}

void RTS::RenderingSystem::Update(CE::World& world, float dt)
{
	StepToCorrectGameState(world, dt);
	UpdateCameraMovement(world, dt);
}

void RTS::RenderingSystem::Render(const CE::World& viewportWorld, CE::RenderCommandQueue& renderQueue) const
{
	if (mCurrentState == nullptr
		|| mPreviousState == nullptr)
	{
		return;
	}

	CE::Renderer::Get().SetClearColor(renderQueue, { 0.5f, 0.3f, 0.15f, 1.0f });

	const RenderingComponent& renderingComponent = viewportWorld.GetRegistry().GetAny<RenderingComponent>();
	const CE::CameraComponent* camera = CE::CameraComponent::TryGetSelectedCamera(viewportWorld);

	if (camera == nullptr)
	{
		return;
	}

	const float totalTimePassed = glm::min(renderingComponent.mTimeStamp, 
		static_cast<float>(mCurrentState->GetNumStepsCompleted()) + 1.f);

	const float interpolationFactor = glm::clamp(
		fmodf(totalTimePassed, Constants::sSimulationStepSize) * (1.0f / Constants::sSimulationStepSize),
		0.0f, 1.0f);

	const auto getColor = [&](TeamId team) -> glm::vec4
		{
			if (team == TeamId::Neutral)
			{
				return glm::vec4{ 0.25f, 0.25f, 0.25f, 1.0f };
			}

			return renderingComponent.mDisplayForTeam == team
				? renderingComponent.mFriendlyColour
				: renderingComponent.mEnemyColour;
		};

	const CE::World& prevWorld = mPreviousState->GetWorld();
	const CE::World& currWorld = mCurrentState->GetWorld();

	auto prevView = prevWorld.GetRegistry().View<CE::TransformedDiskColliderComponent, TeamId, UnitType::Enum>();
	auto currView = currWorld.GetRegistry().View<CE::TransformedDiskColliderComponent, TeamId, UnitType::Enum>();
	float height = 0.0f;
	float heightStep = 1.0f / static_cast<float>(currView.size_hint());

	for (auto [entity, currDisk, team, unitType] : currView.each())
	{
		float renderHeight = height;
		height += heightStep;

		if (!prevView.contains(entity))
		{
			continue;
		}

		const glm::vec4 col = getColor(team);

		const CE::TransformedDiskColliderComponent& prevDisk = prevView.get<CE::TransformedDiskColliderComponent>(entity);

		CE::AddDebugCircle(renderQueue,
			CE::DebugDraw::General,
			CE::To3D(currDisk.mCentre, renderHeight),
			currDisk.mRadius,
			glm::vec4{ 1.0f, 0.0f, 0.0f, 1.0f });

		CE::AddDebugCircle(renderQueue,
			CE::DebugDraw::General,
			CE::To3D(prevDisk.mCentre, renderHeight),
			currDisk.mRadius,
			glm::vec4{ 1.0f, 0.0f, 0.0f, .5f });

		const glm::vec2 deltaPos = currDisk.mCentre - prevDisk.mCentre;
		const glm::vec3 interpolatedPos = CE::To3D(prevDisk.mCentre + deltaPos * interpolationFactor, renderHeight);
		const float interpolatedAngle = CE::Math::Vec2ToAngle(deltaPos);
		
		glm::vec3 orientationEuler{};
		orientationEuler[CE::Axis::Up] = interpolatedAngle - glm::pi<float>();
		glm::quat interpolatedRot{ orientationEuler };

		static constexpr float planeScale = .5f;
		const glm::mat4 worldMat = 
			CE::TransformComponent::ToMatrix(interpolatedPos,
			glm::vec3{ currDisk.mRadius * planeScale, currDisk.mRadius * planeScale, 1.0f },
				interpolatedRot);

		CE::Renderer::Get().AddStaticMesh(renderQueue,
			mMesh,
			mMat,
			worldMat,
			col,
			glm::vec4{ 0.0f });
	}

	for (auto [entity, proj] : prevWorld.GetRegistry().View<ProjectileComponent>().each())
	{
		std::default_random_engine rng{ entt::to_integral(entity) };
		(void)rng();
		(void)rng();
		(void)rng();

		std::uniform_real_distribution dist{ -.5f, .5f };
		const float randomFactor = dist(rng);

		std::uniform_real_distribution endOffsetRange{ -.5f, .5f };
		glm::vec2 randomEndOffset = { endOffsetRange(rng), endOffsetRange(rng) };

		const WeaponType weaponType = GetWeaponType(proj.mFiredFromWeapon);
		const float totalTimeUntilImpact = weaponType.mProjectileTimeUntilImpact;

		auto getTimeAsPercentage = [=](int32 stepsUntilImpact)
			{
				return 1.0f - (((static_cast<float>(stepsUntilImpact) - randomFactor) * Constants::sSimulationStepSize) / totalTimeUntilImpact);
			};

		const float prevTimeAsPercentage = getTimeAsPercentage(proj.mNumStepsUntilImpact);
		const float currTimeAsPercentage = getTimeAsPercentage(proj.mNumStepsUntilImpact - 1);

		const float lerpTimeAsPercentage = CE::Math::lerp(prevTimeAsPercentage, currTimeAsPercentage, interpolationFactor);

		if (lerpTimeAsPercentage < 0.0f
			|| lerpTimeAsPercentage > 1.0f)
		{
			continue;
		}

		const glm::vec2 delta = proj.mTargetPosition + randomEndOffset - proj.mSourcePosition;
		const glm::vec2 interpolatedPos = proj.mSourcePosition + delta * lerpTimeAsPercentage;
		const float interpolatedAngle = CE::Math::Vec2ToAngle(delta);

		glm::vec3 orientationEuler{};
		orientationEuler[CE::Axis::Up] = interpolatedAngle - glm::pi<float>();
		glm::quat interpolatedRot{ orientationEuler };

		const glm::mat4 worldMat =
			CE::TransformComponent::ToMatrix(CE::To3D(interpolatedPos, 2.0f),
				glm::vec3{ .1f, .025f, 1.0f },
				interpolatedRot);

		const glm::vec4 color = proj.mHasHit ? glm::vec4{ 0.0f, 0.0f, 0.0f, 1.0f } : glm::vec4{ 0.0f, 0.0f, 0.0f, .1f };

		CE::Renderer::Get().AddStaticMesh(renderQueue,
			mMesh,
			mMat,
			worldMat,
			color,
			glm::vec4{ 0.0f });

		CE::AddDebugLine(
			renderQueue,
			CE::DebugDraw::Gameplay,
			CE::To3D(proj.mSourcePosition),
			CE::To3D(proj.mTargetPosition),
			glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });
	}


	for (auto [entity, disk, objective, team] : currWorld.GetRegistry().View<CE::TransformedDiskColliderComponent,
			ObjectiveComponent,
			TeamId>().each())
	{
		CE::AddDebugCircle(renderQueue,
			CE::DebugDraw::Sound,
			CE::To3D(disk.mCentre),
			disk.mRadius,
			getColor(team));

		const glm::vec4 ringCol = getColor(objective.mAmountCaptured < 0.0f ? TeamId::Team1 : TeamId::Team2);

		static constexpr int numSteps = 25;

		for (int i = 0; i < numSteps; i++)
		{
			const float percentage = static_cast<float>(i) / static_cast<float>(numSteps);

			if (percentage > glm::abs(objective.mAmountCaptured))
			{
				break;
			}

			float radius = percentage * disk.mRadius;

			CE::AddDebugCircle(renderQueue,
				CE::DebugDraw::Sound,
				CE::To3D(disk.mCentre, .1f),
				radius,
				ringCol);
		}
	}
}

void RTS::RenderingSystem::StepToCorrectGameState(CE::World& viewportWorld, float dt)
{
	RenderingComponent& renderingComponent = viewportWorld.GetRegistry().GetAny<RenderingComponent>();
	SimulationComponent& simulationComponent = viewportWorld.GetRegistry().GetAny<SimulationComponent>();

	renderingComponent.mTimeStamp = glm::max(0.0f, renderingComponent.mTimeStamp + renderingComponent.mPlaySpeed * dt);

	auto updateSteps = [&](std::unique_ptr<GameState>& state, int requiredNumSteps)
		{
			if (state == nullptr
				|| static_cast<int>(state->GetNumStepsCompleted()) > requiredNumSteps)
			{
				state = std::make_unique<GameState>(simulationComponent.mTeam1Script, simulationComponent.mTeam2Script);
				state = std::make_unique<GameState>(simulationComponent.mTeam1Script, simulationComponent.mTeam2Script);
			}

			if (static_cast<int>(state->GetNumStepsCompleted()) != requiredNumSteps)
			{
				std::lock_guard guard{ mRenderingQueueMutex };

				for (int i = static_cast<int>(state->GetNumStepsCompleted()); i < requiredNumSteps; i++)
				{
					state->Step(mRenderingQueue[i]);
				}
			}
		};

	int currRequiredNumSteps = glm::clamp((static_cast<int>(renderingComponent.mTimeStamp / Constants::sSimulationStepSize) + 2),
		1,
		static_cast<int>(mRenderingQueue.size()));

	updateSteps(mPreviousState, currRequiredNumSteps - 1);
	updateSteps(mCurrentState, currRequiredNumSteps);
}

void RTS::RenderingSystem::UpdateCameraMovement(CE::World& world, float dt) const
{
	static constexpr float movementSpeed = 2.0f;
	static constexpr float zoomFactor = 1.2f;

	auto camView = world.GetRegistry().View<CE::TransformComponent, CE::CameraComponent>();
	entt::entity camEntity = CE::CameraComponent::GetSelected(world);
	ASSERT(camView.contains(camEntity));

	auto [transform, camera] = camView.get(camEntity);
	CE::Input& input = CE::Input::Get();

	const glm::vec2 movementKeys
	{
		input.GetKeyboardAxis(CE::Input::KeyboardKey::W, CE::Input::KeyboardKey::S),
		input.GetKeyboardAxis(CE::Input::KeyboardKey::D, CE::Input::KeyboardKey::A),
	};

	glm::vec3 cameraPos = transform.GetWorldPosition();
	cameraPos += CE::To3D(movementKeys * dt * (movementSpeed * camera.mOrthoGraphicSize));

	const float zoomInput = input.GetMouseWheel();

	if (zoomInput > 0.0f)
	{
		camera.mOrthoGraphicSize /= zoomFactor;
	}
	else if (zoomInput < 0.0f)
	{
		camera.mOrthoGraphicSize *= zoomFactor;
	}

	const float cameraHeight = cameraPos[CE::Axis::Up];
	camera.mNear = cameraHeight - 5.0f;
	camera.mFar = cameraHeight + 5.0f;

	transform.SetWorldPosition(cameraPos);
}