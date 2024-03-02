#pragma once

#include "AudioBuffer.h"

#include <Engine/Core/Ref.h>

#include <glm.hpp>

//Forward Declaration(s)
namespace gte{
	class Scene;
	struct SpeakerComponent;
}

namespace gte::audio {

	class ENGINE_API Source {
	public:
		Source(void);
		~Source(void);

		//Functions to handle playback
		void Play(void);
		void Stop(void);
		void Pause(void);

		bool IsPlaying(void) const;
		bool IsPaused(void) const;

	private:

		//Set Properties values
		void SetProperties(const SpeakerComponent* speaker);
		void SetPosition(const glm::vec3& pos) const;
		void SetVelocity(const glm::vec2& velocity) const;

	private:
		Ref<uint32> mSourceID = CreateRef<uint32>(0);
		uint32 mAttachedBuffer = 0;
		bool mShouldStart = false;
		friend class gte::Scene;
	};

}