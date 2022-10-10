#include "AudioBuffer.h"

#include <AL/al.h>

namespace gte::audio {

	AudioBuffer::AudioBuffer(const void* data, size_t size, int32 format, int32 samplerate)
	{
		alGenBuffers(1, &mBufferID);
		alBufferData(mBufferID, format, data, static_cast<int32>(size), samplerate);
		const ALenum result = alGetError();
		if (result != AL_NO_ERROR)
		{
			GTE_TRACE_LOG("Setting alBuffer resulted in error: ", result);
		}
	}
	
	void AudioBuffer::Load(const void* data, size_t size, int32 format, int32 samplerate) { alBufferData(mBufferID, format, data, static_cast<int32>(size), samplerate); }

	AudioBuffer::~AudioBuffer(void) { alDeleteBuffers(1, &mBufferID); }
	AudioBuffer::AudioBuffer(void) { alGenBuffers(1, &mBufferID); }

}