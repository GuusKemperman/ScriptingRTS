#ifdef EDITOR
#pragma once
#include "EditorSystems/EditorSystem.h"

#include <shared_mutex>

#include "Scripting/CompiledProgram.h"
#include "Core/PlayerDataBase.h"
#include "Rendering/FrameBuffer.h"

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

		CE::FrameBuffer mViewportFrameBuffer{};
		std::shared_ptr<const PlayerDataBase::Player> mCurrentlyWatchedPlayer{};
		std::unique_ptr<CE::World> mCurrentlyWatchedWorld{};

		PlayerDataBase mPlayerDataBase{};

		std::jthread mThread{};
	};
}
#endif // EDITOR