#include "Source.h"

#include <Engine/Scene/Components.h>

#include <AL/al.h>

namespace gte::audio {

	void Source::Play(void)
	{
		if (mAttachedBuffer)
		{
			alSourcePlay(*mSourceID);
			mShouldStart = false;
		}
		else
			mShouldStart = true;
	}

	void Source::Pause(void)
	{
		alSourcePause(*mSourceID);
		mShouldStart = false;
	}

	void Source::Stop(void)
	{
		alSourceStop(*mSourceID);
		mShouldStart = false;
	}

	bool Source::IsPlaying(void) const
	{
		int32 result;
		alGetSourcei(*mSourceID, AL_SOURCE_STATE, &result);
		return result == AL_PLAYING;
	}

	void Source::SetProperties(const SpeakerComponent* speaker)
	{
		alSourcef(*mSourceID, AL_GAIN, speaker->Volume);
		alSourcef(*mSourceID, AL_PITCH, speaker->Pitch);
		alSourcef(*mSourceID, AL_ROLLOFF_FACTOR, speaker->RollOffFactor);
		alSourcef(*mSourceID, AL_REFERENCE_DISTANCE, speaker->RefDistance);
		alSourcef(*mSourceID, AL_MAX_DISTANCE, speaker->MaxDistance);
		alSourcei(*mSourceID, AL_LOOPING, speaker->Looping ? AL_TRUE : AL_FALSE);

		AudioBuffer* buffer = (AudioBuffer*)speaker->AudioClip->Data;
		if (speaker->AudioClip->Type == AssetType::AUDIO && mAttachedBuffer != buffer->GetID())
		{
			alSourcei(*mSourceID, AL_BUFFER, buffer->GetID());
			mAttachedBuffer = buffer->GetID();
			if (mShouldStart)
				Play();
		}
		else if(speaker->AudioClip->Type != AssetType::AUDIO)
		{
			alSourcei(*mSourceID, AL_BUFFER, 0);
			mAttachedBuffer = 0;
			//mShouldStart = false;
		}
	}

	void Source::SetPosition(const glm::vec3& pos) const { alSource3f(*mSourceID, AL_POSITION, pos.x, pos.y, pos.z); }
	void Source::SetVelocity(const glm::vec2& velocity) const { alSource3f(*mSourceID, AL_VELOCITY, velocity.x, velocity.y, 0.0f); }

	Source::Source(void) { alGenSources(1, mSourceID.get()); }
	Source::~Source(void)
	{
		if(mSourceID.use_count() == 1)
			alDeleteSources(1, mSourceID.get());
	}
}