#include "AudioDevice.h"

#include <Engine/Core/Context.h>

#ifndef GT_WEB
	#include <AL/alc.h>
#endif

namespace gte::audio {

	AudioDevice::AudioDevice(void)
	{
		//Opening default audio device
		mALCDevice = alcOpenDevice(nullptr);
		ASSERT(mALCDevice, "Failed to get sound device");

		//Create Context for the device
		mALCContext = alcCreateContext(mALCDevice, nullptr);
		
		const ALCboolean result = alcMakeContextCurrent(mALCContext);
		ASSERT(result, "Failed to make context current");

		const ALCchar* name = nullptr;
		if (alcIsExtensionPresent(mALCDevice, "ALC_ENUMERATE_ALL_EXT"))
			name = alcGetString(mALCDevice, ALC_ALL_DEVICES_SPECIFIER);
		if (!name || alcGetError(mALCDevice) != ALC_NO_ERROR)
			name = alcGetString(mALCDevice, ALC_DEVICE_SPECIFIER);

		GTE_INFO_LOG("Opened \"", name, "\"");
	}

	AudioDevice::~AudioDevice(void)
	{
		ALCboolean result = alcMakeContextCurrent(nullptr);
		ASSERT(result, "Failed to set context to null");
		
		alcDestroyContext(mALCContext);
		result = alcCloseDevice(mALCDevice);
		ASSERT(result, "Failed to close audio device");
	}

	[[nodiscard]] AudioDevice* AudioDevice::Get(void)
	{
		if (!internal::GetContext()->AudioDevice)
			internal::GetContext()->AudioDevice = new AudioDevice();

		return internal::GetContext()->AudioDevice;
	}

}