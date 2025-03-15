#include "Precomp.h"
#include "Scripting/CompiledProgram.h"

#ifdef EDITOR
void RTS::CompiledInstruction::DisplayWidget(const std::string& name)
{
	CE::ShowInspectUI(name, mAction);
	CE::ShowInspectUI(name, mFilter);
}
#endif // EDITOR

void RTS::CompiledProgram::Run() const
{
	uint32 instruction{};

	while (instruction < mInstructions.size())
	{
		const CompiledInstruction& current = mInstructions[instruction];
		if (RTSAPI::Action(current.mAction, current.mFilter))
		{
			break;
		}

		++instruction;
	}
}
