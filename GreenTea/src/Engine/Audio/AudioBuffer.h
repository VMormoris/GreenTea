#pragma once

#include <Engine/Core/Engine.h>

namespace gte::audio {

	class ENGINE_API AudioBuffer {
	public:

		AudioBuffer(void);
		AudioBuffer(const void* data, size_t size, int32 format, int32 samplerates);
		~AudioBuffer(void);

		void Load(const void* data, size_t size, int32 format, int32 samplerates);

		[[nodiscard]] uint32 GetID(void) const noexcept { return mBufferID; }

	private:

		uint32 mBufferID;

	};

}