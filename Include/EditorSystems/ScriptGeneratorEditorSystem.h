#ifdef EDITOR
#pragma once
#include "EditorSystems/EditorSystem.h"

#include <shared_mutex>

#include "Scripting/CompiledProgram.h"

namespace RTS
{
	class ScriptGeneratorEditorSystem final :
		public CE::EditorSystem
	{
	public:
		ScriptGeneratorEditorSystem();

		void Tick(float deltaTime) override;

	private:
		friend CE::ReflectAccess;
		static CE::MetaType Reflect();
		REFLECT_AT_START_UP(ScriptGeneratorEditorSystem);

		void SimulateThread(const std::stop_token& stop);

		std::shared_mutex mBestProgramMutex{};
		CompiledProgram mBestProgram{};

		std::jthread mThread{};
	};
}
#endif // EDITOR