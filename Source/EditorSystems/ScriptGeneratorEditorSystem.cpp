#include "Precomp.h"
#include "EditorSystems/ScriptGeneratorEditorSystem.h"

#include <execution>

#include "Components/SimulationComponent.h"
#include "Utilities/Random.h"

RTS::ScriptGeneratorEditorSystem::ScriptGeneratorEditorSystem() :
	EditorSystem("ScriptGeneratorEditorSystem"),
	mThread([this](const std::stop_token& token) { SimulateThread(token); })
{
}

void RTS::ScriptGeneratorEditorSystem::Tick(float deltaTime)
{
	EditorSystem::Tick(deltaTime);

	if (Begin())
	{
		CompiledProgram bestProgram = [this]()
			{
				std::shared_lock l{ mBestProgramMutex };
				return mBestProgram;
			}();

		CE::ShowInspectUI("BestProgram", bestProgram.mInstructions);
		End();
	}
}

void RTS::ScriptGeneratorEditorSystem::SimulateThread(const std::stop_token& stop)
{
	auto isTeam1Better = [&](const CompiledProgram& team1, const CompiledProgram& team2)
		{
			float currentProgramScore{};
			float bestProgramScore{};

			SimulationComponent sim{};
			sim.mUseMultiThreading = false;
			sim.mRunOnCallingThread = true;
			sim.mUsePhysics = false;
			sim.mStartingTotalNumOfUnits = 2;

			sim.mInvokeEvaluateEvents =
				[&]()
				{
					const CE::Registry& reg = sim.GetGameState().GetWorld().GetRegistry();

					for (entt::entity entity : reg.View<Team1Tag>())
					{
						sim.OnPreEvaluate(entity);
						team1.Run();
					}

					for (entt::entity entity : reg.View<Team2Tag>())
					{
						sim.OnPreEvaluate(entity);
						team2.Run();
					}
				};

			sim.StartSimulation();

			currentProgramScore = sim.GetGameState().GetScore(TeamId::Team1);
			bestProgramScore = sim.GetGameState().GetScore(TeamId::Team2);

			return currentProgramScore > bestProgramScore;
		};

	auto randomizeProgram = [](CompiledProgram& program)
		{
			program.mInstructions.clear();
			program.mInstructions.resize(CE::Random::Range<size_t>(1, 32));

			static constexpr auto randomEnum = []<typename T>(T & value)
			{
				static constexpr auto values = magic_enum::enum_values<T>();
				size_t index = CE::Random::Range<size_t>(0, values.size());
				value = values[index];
			};

			for (CompiledInstruction& instruction : program.mInstructions)
			{
				randomEnum(instruction.mAction);
				randomEnum(instruction.mFilter.mTeam);
				randomEnum(instruction.mFilter.mSortByDistance);
				randomEnum(instruction.mFilter.mRange);
				randomEnum(instruction.mFilter.mHealth);
			}
		};

	std::array<CompiledProgram, 16> programs{};
	std::for_each(std::execution::par_unseq, programs.begin(), programs.end(),
		[&](CompiledProgram& program)
		{
			randomizeProgram(program);

			while (!stop.stop_requested())
			{
				CompiledProgram bestProgram = [this]()
					{
						std::shared_lock l{ mBestProgramMutex };
						return mBestProgram;
					}();

				if (!isTeam1Better(program, bestProgram))
				{
					randomizeProgram(program);
					continue;
				}

				std::unique_lock l{ mBestProgramMutex };

				if (bestProgram == mBestProgram)
				{
					mBestProgram = program;
				}
			}
		});
}


CE::MetaType RTS::ScriptGeneratorEditorSystem::Reflect()
{
	CE::MetaType type = {
		CE::MetaType::T<ScriptGeneratorEditorSystem>{},
		"ScriptGeneratorEditorSystem",
		CE::MetaType::Base<EditorSystem>{} };
	return type;
}