#include "Precomp.h"
#include "States/CaptureObjectiveState.h"

#include "Components/ObjectiveComponent.h"
#include "Components/Physics2D/DiskColliderComponent.h"
#include "Core/GameSimulationStep.h"
#include "Core/GameState.h"
#include "Core/RTSCollisionLayers.h"
#include "World/Physics.h"
#include "World/Registry.h"

void RTS::CaptureObjectiveState::Execute(const GameState& gameState, GameSimulationStep& nextStep,
	std::span<const CaptureObjectiveState>)
{
	const CE::Registry& reg = gameState.GetWorld().GetRegistry();

	for (auto [entity, disk, objective] : reg.View<CE::TransformedDiskColliderComponent, ObjectiveComponent>().each())
	{
		auto getAmountOnObjective = [&](TeamId team)
			{
				uint32 count{};
				CE::CollisionRules filter{};
				filter.mLayer = CE::CollisionLayer::Query;
				filter.SetResponse(ToCE(*GetCollisionLayer(team)), CE::CollisionResponse::Overlap);

				gameState.GetWorld().GetPhysics().Query(disk,
					filter,
					[&]([[maybe_unused]] const auto& shape, [[maybe_unused]] entt::entity entity)
					{
						++count;
					});

				return count;
			};

		nextStep.AddCommand(CaptureObjectiveCommand{ entity, getAmountOnObjective(TeamId::Team1), getAmountOnObjective(TeamId::Team2) });
	}
}

