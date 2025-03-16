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

		for (size_t i = 0; i < topPlayers.size(); i++)
		{
			const PlayerDataBase::Player& player = *topPlayers[i];

			ImGui::PushID(player.mName.c_str());
			ImGui::Text("%i - ELO: %f | %s", static_cast<int>(i + 1), player.mElo, player.mName.c_str());
			ImGui::SameLine();

			if (ImGui::CollapsingHeader("Program"))
			{
				// TODO read-only inspecting
				CE::ShowInspectUI("Instructions", const_cast<CompiledProgram&>(player.mProgram).mInstructions);
			}
			ImGui::PopID();
		}

		size_t numberOfPlayers = mPlayerDataBase.GetNumberOfPlayers();

		if (numberOfPlayers > 1000)
		{
			float eraseAllWithELOLowerThan = topPlayers.back()->mElo;

			auto [lock, players] = mPlayerDataBase.OpenForWrite();

			std::erase_if(players, [&](const std::shared_ptr<PlayerDataBase::Player>& player)
				{
					return player->mElo < eraseAllWithELOLowerThan;
				});
		}

		End();
	}
}

void RTS::ScriptGeneratorEditorSystem::SimulateThread(const std::stop_token& stop)
{
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

	std::array<CompiledProgram, 16> programs{};
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