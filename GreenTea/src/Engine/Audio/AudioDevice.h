#pragma once

#include <Engine/Core/Engine.h>

#ifndef GT_WEB
	//Forward declaration(s)
	struct ALCdevice;
	struct ALCcontext;
#else
	#include <AL/alc.h>
#endif

namespace gte::audio {

	class ENGINE_API AudioDevice {
	public:
		~AudioDevice(void);

		[[nodiscard]] static AudioDevice* Get(void);

	private:

		AudioDevice(void);

	private:

		ALCdevice* mALCDevice;
		ALCcontext* mALCContext;

	};

}