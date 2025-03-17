#pragma once
#include "Core/ScriptingAPI.h"
#include "Core/UnitFilter.h"

namespace RTS
{
	struct CompiledInstruction
	{
#ifdef EDITOR
		void DisplayWidget(const std::string& name);
#endif

		bool operator==(const CompiledInstruction& other) const;
		bool operator!=(const CompiledInstruction& other) const;

		union
		{
			Action mAction{};
			uint16 mAmountToJumpIfTrue;
		};
		UnitFilter mFilter{};

		enum class Type : uint8
		{
			Action,
			Condition
		};
		Type mType = Type::Action;
	};

	struct CompiledProgram
	{
		void Run() const;

		bool operator==(const CompiledProgram&) const = default;
		bool operator!=(const CompiledProgram&) const = default;

		static void swap(CompiledProgram& lhs, CompiledProgram& rhs)
		{
			std::swap(lhs.mInstructions, rhs.mInstructions);
		}

		std::vector<CompiledInstruction> mInstructions{};
	};
}
#ifdef EDITOR
IMGUI_AUTO_DEFINE_INLINE(template<>, RTS::CompiledInstruction, var.DisplayWidget(name);)
#endif // EDITOR
