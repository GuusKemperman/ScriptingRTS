#include "Precomp.h"
#include "Core/Engine.h"

int main(int argc, char* argv[])
{
	CE::EngineConfig config{ argc, argv };
	config.mGameDir = GAME_DIR;

	CE::Engine engine{ config };
	engine.Run("L_Training");

	return 0;
}
