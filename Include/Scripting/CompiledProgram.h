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

		bool operator==(const CompiledInstruction&) const = default;
		bool operator!=(const CompiledInstruction&) const = default;

		Action mAction{};
		UnitFilter mFilter{};
	};

	struct CompiledProgram
	{
		void Run() const;

		bool operator==(const CompiledProgram&) const = default;
		bool operator!=(const CompiledProgram&) const = default;

		std::vector<CompiledInstruction> mInstructions{};
	};
}

#ifdef EDITOR
IMGUI_AUTO_DEFINE_INLINE(template<>, RTS::CompiledInstruction, var.DisplayWidget(name);)
#endif // EDITOR
