#include "Precomp.h"
#include "EditorSystems/ScriptGeneratorEditorSystem.h"

#include <execution>

#include "Assets/Level.h"
#include "Components/SimulationComponent.h"
#include "Core/AssetManager.h"
#include "Utilities/Random.h"
#include "Utilities/Imgui/ImguiHelpers.h"
#include "Utilities/Imgui/WorldViewportPanel.h"

using namespace magic_enum::bitwise_operators;

namespace
{
	RTS::CompiledProgram AttackAndMoveOnly()
	{
		using namespace RTS;
		CompiledProgram program{};
		program.mInstructions.resize(2);

		program.mInstructions[0].mAction = Action::ShootAt;
		program.mInstructions[0].mFilter.mTeam.mEnum = TeamFilter::Enemy;
		program.mInstructions[1].mFilter.mRange.mEnum = RangeFilter::ShortRange | RangeFilter::MidRange | RangeFilter::LongRange;

		program.mInstructions[1].mAction = Action::MoveTo;
		program.mInstructions[1].mFilter.mTeam.mEnum = TeamFilter::Enemy;
		return program;
	}

	RTS::CompiledProgram FleeAttackAndMoveOnly()
	{
		using namespace RTS;
		CompiledProgram program{};
		program.mInstructions.resize(4);

		program.mInstructions[0].mType = CompiledInstruction::Type::Condition;
		program.mInstructions[0].mFilter.mTeam.mEnum = TeamFilter::Myself;
		program.mInstructions[0].mFilter.mHealth.mEnum = HealthFilter::From0To25 | HealthFilter::From25To50;
		program.mInstructions[0].mAmountToJumpIfTrue = 3;

		program.mInstructions[1].mAction = Action::ShootAt;
		program.mInstructions[1].mFilter.mTeam.mEnum = TeamFilter::Enemy;
		program.mInstructions[1].mFilter.mRange.mEnum = RangeFilter::ShortRange | RangeFilter::MidRange | RangeFilter::LongRange;

		program.mInstructions[2].mAction = Action::MoveTo;
		program.mInstructions[2].mFilter.mTeam.mEnum = TeamFilter::Enemy;

		program.mInstructions[3].mAction = Action::FleeFrom;
		program.mInstructions[3].mFilter.mTeam.mEnum = TeamFilter::Enemy;

		return program;
	}
}

RTS::ScriptGeneratorEditorSystem::ScriptGeneratorEditorSystem() :
	EditorSystem("ScriptGeneratorEditorSystem")
{
	mPlayerDataBase.AddPlayer(AttackAndMoveOnly(), "AttackAndMoveOnly");
	mPlayerDataBase.AddPlayer(FleeAttackAndMoveOnly(), "FleeAttackAndMoveOnly");
}

void RTS::ScriptGeneratorEditorSystem::Tick(float deltaTime)
{
	EditorSystem::Tick(deltaTime);

	if (Begin())
	{
		if (!mThread.joinable() && ImGui::Button("Start"))
		{
			mThread = std::jthread{ [this](const std::stop_token& token) { SimulateThread(token); } };
		}

		std::vector<std::shared_ptr<PlayerDataBase::Player>> topPlayers(100);

		{
			auto [lock, allPlayers] = mPlayerDataBase.OpenForRead();
			auto end = std::partial_sort_copy(
				allPlayers.begin(), allPlayers.end(),
				topPlayers.begin(), topPlayers.end(),
				[](const std::shared_ptr<PlayerDataBase::Player>& lhs, const std::shared_ptr<PlayerDataBase::Player>& rhs)
				{
					return lhs->mElo > rhs->mElo;
				});

			topPlayers.erase(end, topPlayers.end());
		}

		static float viewportWidth = .75f;
		static float scoreWidth = .25f;
		ImGui::Splitter(true, &scoreWidth, &viewportWidth);

		if (ImGui::BeginChild("Scoreboard", { scoreWidth, -2.0f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav))
		{
			for (size_t i = 0; i < topPlayers.size(); i++)
			{
				const PlayerDataBase::Player& player = *topPlayers[i];

				ImGui::PushID(player.mName.c_str());
				ImGui::TextUnformatted(CE::Format("{} - ELO: {} - Games: {} | {}", 
					static_cast<int>(i + 1), 
					player.mElo, 
					player.mGamesPlayed,
					player.mName).c_str());
				ImGui::SameLine();

				if (ImGui::CollapsingHeader("Program"))
				{
					if (ImGui::Button("Show"))
					{
						mCurrentlyWatchedPlayer = topPlayers[i];
						mCurrentlyWatchedWorld.reset();
					}
					// TODO read-only inspecting
					CE::ShowInspectUI("Instructions", const_cast<CompiledProgram&>(player.mProgram).mInstructions);
				}
				ImGui::PopID();
			}
		}
		ImGui::EndChild();
		ImGui::SameLine();

		if (ImGui::BeginChild("BattleTV", { viewportWidth, -2.0f }, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoNav))
		{
			if (mCurrentlyWatchedPlayer != nullptr)
			{
				if (mCurrentlyWatchedWorld == nullptr)
				{
					CE::AssetHandle level = CE::AssetManager::Get().TryGetAsset<CE::Level>("L_SimulationConfig");
					ASSERT(level != nullptr);
					mCurrentlyWatchedWorld = level->CreateWorld(false);

					CE::Registry& reg = mCurrentlyWatchedWorld->GetRegistry();
					SimulationComponent& sim = reg.GetAny<SimulationComponent>();

					sim.mUsePhysics = true;
					sim.mStartingTotalNumOfUnits = 16;

					std::shared_ptr<const PlayerDataBase::Player> opponent = mPlayerDataBase.FindMatch(*mCurrentlyWatchedPlayer);

					if (opponent == nullptr)
					{
						opponent = mCurrentlyWatchedPlayer;
					}

					sim.SetCompiledScripts(mCurrentlyWatchedPlayer->mProgram, FleeAttackAndMoveOnly());
					LOG(LogGame, Message, "{} ({}) vs {} ({})",
						mCurrentlyWatchedPlayer->mName, mCurrentlyWatchedPlayer->mElo,
						opponent->mName, opponent->mElo);

					mCurrentlyWatchedWorld->BeginPlay();
				}

				mCurrentlyWatchedWorld->Tick(deltaTime);
				CE::WorldViewportPanel::Display(*mCurrentlyWatchedWorld, mViewportFrameBuffer, nullptr);
			}
		}
		ImGui::EndChild();


		size_t numberOfPlayers = mPlayerDataBase.GetNumberOfPlayers();

		if (numberOfPlayers > 200)
		{
			float eraseAllWithELOLowerThan = topPlayers.back()->mElo;

			auto [lock, players] = mPlayerDataBase.OpenForWrite();

			std::erase_if(players, [&](const std::shared_ptr<PlayerDataBase::Player>& player)
				{
					return player->mElo < eraseAllWithELOLowerThan;
				});
		}
	}
	End();
}

void RTS::ScriptGeneratorEditorSystem::SimulateThread(const std::stop_token& stop)
{
	auto randomizeProgram = [](CompiledProgram& program)
		{
			program.mInstructions.clear();
			program.mInstructions.resize(CE::Random::Range<size_t>(3, 16));

			static constexpr auto randomEnum = []<typename T>(T& value)
			{
				static constexpr auto values = magic_enum::enum_values<T>();
				size_t index = CE::Random::Range<size_t>(0, values.size());
				value = values[index];
			};

			static constexpr auto randomBitEnum = []<typename T>(T& value)
			{
				value = static_cast<T>(CE::Random::Value<uint32>());
			};

			for (size_t i = 0; i < program.mInstructions.size(); i++)
			{
				CompiledInstruction& instruction = program.mInstructions[i];
				randomEnum(instruction.mType);

				switch (instruction.mType)
				{
				case CompiledInstruction::Type::Action:
				{
					randomEnum(instruction.mAction);
					break;
				}
				case CompiledInstruction::Type::Condition:
				{
					instruction.mAmountToJumpIfTrue = static_cast<decltype(instruction.mAmountToJumpIfTrue)>
						(CE::Random::Range<size_t>(0, program.mInstructions.size() - i));
					break;
				}
				}
				randomEnum(instruction.mFilter.mSortByDistance);
				randomBitEnum(instruction.mFilter.mType.mEnum);
				randomBitEnum(instruction.mFilter.mTeam.mEnum);
				randomBitEnum(instruction.mFilter.mRange.mEnum);
				randomBitEnum(instruction.mFilter.mHealth.mEnum);
			}
		};

	auto randomName = []()
		{
			std::string output = "The ";

			static constexpr std::array adjectives =
			{
				"great", "amazing", "tragic", "horrible", "gruesome", "deadly", "beautiful", "historic", "unnecessary", "necessary", "unforeseen", "long awaited"
			};
			output = adjectives[CE::Random::Range<size_t>(0, adjectives.size())];
			output.push_back(' ');

			static constexpr std::array vowelishSyllables = { "a", "e", "u", "i", "o", "oo", "ea" };
			static constexpr std::array consonantishSyllable = { "w", "r", "t", "y", "p", "s", "d", "f", "g", "j", "k", "br", "ch", "rt", "tr", "pr", "dr", "sh", "ng" };

			bool lastSylableWasVowel = CE::Random::Value<bool>();
			const size_t numSylables = CE::Random::Range(5, 13);
			for (size_t i = 0; i < numSylables; i++)
			{
				const char* whatToAppendThisCycle{};
				if (lastSylableWasVowel)
				{
					whatToAppendThisCycle = consonantishSyllable[CE::Random::Range<size_t>(0, consonantishSyllable.size())];
				}
				else
				{
					whatToAppendThisCycle = vowelishSyllables[CE::Random::Range<size_t>(0, vowelishSyllables.size())];
				}
				output.append(whatToAppendThisCycle);
				lastSylableWasVowel = !lastSylableWasVowel;
			}

			return output;
		};

	std::array<CompiledProgram, 7> programs{};
	std::for_each(std::execution::par_unseq, programs.begin(), programs.end(),
		[&](CompiledProgram& program)
		{
			while (!stop.stop_requested())
			{
				randomizeProgram(program);
				mPlayerDataBase.AddPlayer(program, randomName());
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