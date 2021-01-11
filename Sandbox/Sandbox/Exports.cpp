#include "Exports.h"

extern "C" {

	GAME_API void* CreatePlayer(void)
	{
		return static_cast<void*>(new Player());
	}

	GAME_API void* CreateCameraController(void)
	{
		return static_cast<void*>(new CameraController());
	}

}