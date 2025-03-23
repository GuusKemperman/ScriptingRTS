#include "Precomp.h"
#include "Commands/CaptureObjectiveCommand.h"

#include "Components/ObjectiveComponent.h"
#include "Core/Constants.h"
#include "Core/GameState.h"
#include "World/Registry.h"

void RTS::CaptureObjectiveCommand::Execute(GameState& state, std::span<const CaptureObjectiveCommand> commands)
{
	CE::Registry& reg = state.GetWorld().GetRegistry();

	auto view = reg.View<ObjectiveComponent, TeamId>();

	for (const auto& [entity, numTeam1, numTeam2] : commands)
	{
		auto [objective, objectiveTeam] = view.get(entity);

		auto setNewCaptureValue = [&](float value)
			{
				const float oldValue = objective.mAmountCaptured;
				objective.mAmountCaptured = value = glm::clamp(value, -1.0f, 1.0f);

				auto captureByTeam = [&](TeamId team)
					{
						auto* oldStorage = reg.Storage(GetTeamTagStorage(objectiveTeam));
						ASSERT(oldStorage != nullptr);
						oldStorage->remove(entity);

						objectiveTeam = team;

						auto* newStorage = reg.Storage(GetTeamTagStorage(objectiveTeam));
						ASSERT(newStorage != nullptr);
						newStorage->push(entity);
					};

				if (oldValue != -1.0f 
					&& value == -1.0f)
				{
					captureByTeam(TeamId::Team1);
				}
				else if (oldValue != 1.0f
					&& value == 1.0f)
				{
					captureByTeam(TeamId::Team2);
				}
				else if ((oldValue > 0.0f) != (value > 0.0f))
				{
					// moved across 0.0f, objective becomes neutral
					captureByTeam(TeamId::Neutral);
				}
			};

		constexpr auto teamToAmountCaptured = [](TeamId team)
			{
				switch (team)
				{
				case TeamId::Team1: return -1.0f;
				case TeamId::Neutral: return 0.0f;
				case TeamId::Team2: return 1.0f;
				}
				return .0f;
			};

		if (objectiveTeam != TeamId::Neutral && teamToAmountCaptured(objectiveTeam) == objective.mAmountCaptured)
		{
			state.SetScore(objectiveTeam, 
				state.GetScore(objectiveTeam) + ObjectiveComponent::sPointsPerSecond * Constants::sSimulationStepSize);
		}

		if (numTeam1 == 0
			&& numTeam2 == 0)
		{
			// Slowly return to last completed capture state
			const float desiredState = teamToAmountCaptured(objectiveTeam);
			const float delta = desiredState - objective.mAmountCaptured;

			if (delta != 0.0f)
			{
				const float normalisedDelta = delta / delta;
				const float deltaWithSpeed = normalisedDelta * Constants::sSimulationStepSize * ObjectiveComponent::sPassiveAmountCapturedLossPerSecond;
				
				if (glm::abs(deltaWithSpeed) > glm::abs(delta))
				{
					setNewCaptureValue(objective.mAmountCaptured + delta);
				}
				else
				{
					setNewCaptureValue(objective.mAmountCaptured + deltaWithSpeed);
				}
			}
			continue;
		}

		if (numTeam1 == numTeam2)
		{
			continue;
		}

		auto getCaptureSpeed = [](uint32 count)
			{
				if (count == 0)
				{
					return 0.0f;
				}
				if (count == 1)
				{
					return 1.0f / ObjectiveComponent::sCaptureDuration;
				}
				return (1.0f + static_cast<float>(count - 1) * 0.5f) / ObjectiveComponent::sCaptureDuration;
			};

		const float team1Speed = getCaptureSpeed(numTeam1);
		const float team2Speed = getCaptureSpeed(numTeam2);

		const float team1Delta = teamToAmountCaptured(TeamId::Team1) * team1Speed;
		const float team2Delta = teamToAmountCaptured(TeamId::Team2) * team2Speed;

		setNewCaptureValue(objective.mAmountCaptured + team1Delta + team2Delta);
	}
}
