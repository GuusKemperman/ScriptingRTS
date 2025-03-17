#include "Precomp.h"
#include "Scripting/CompiledProgram.h"

#ifdef EDITOR
void RTS::CompiledInstruction::DisplayWidget(const std::string&)
{
	switch (mType)
	{
	case Type::Action:CE::ShowInspectUI("Action", mAction); break;
	case Type::Condition:
	{
		uint32 tmp = mAmountToJumpIfTrue;
		CE::ShowInspectUI("AmountToJumpIfTrue", tmp);
		mAmountToJumpIfTrue = static_cast<decltype(mAmountToJumpIfTrue)>(tmp);
		break;
	}
	}
	
	CE::ShowInspectUI("Target", mFilter);
}

bool RTS::CompiledInstruction::operator==(const CompiledInstruction& other) const
{
	if (mType != other.mType
		|| mFilter != other.mFilter)
	{
		return false;
	}

	switch (mType)
	{
	case Type::Action: return mAction == other.mAction;
	case Type::Condition: return mAmountToJumpIfTrue == other.mAmountToJumpIfTrue;
	}
	return true;
}

bool RTS::CompiledInstruction::operator!=(const CompiledInstruction& other) const
{
	return !(*this == other);
}
#endif // EDITOR

void RTS::CompiledProgram::Run() const
{
	uint32 instruction{};

	while (instruction < mInstructions.size())
	{
		const CompiledInstruction& current = mInstructions[instruction];

		switch (current.mType)
		{
		case CompiledInstruction::Type::Action:
		{
			if (RTSAPI::Action(current.mAction, current.mFilter))
			{
				return;
			}
			++instruction;
			break;
		}
		case CompiledInstruction::Type::Condition:
		{
			if (RTSAPI::Condition(current.mFilter))
			{
				instruction += current.mAmountToJumpIfTrue;
			}
			else
			{
				instruction++;
			}
			break;
		}
		}
	}
}
