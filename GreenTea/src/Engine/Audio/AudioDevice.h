#pragma once

#include <Engine/Core/Engine.h>

//Forward declaration(s)
struct ALCdevice;
struct ALCcontext;

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