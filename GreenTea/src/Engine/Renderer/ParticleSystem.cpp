#include "ParticleSystem.h"
#include "Renderer2D.h"

#include <Engine/Core/Random.h>

#include <entt.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <gtx/compatibility.hpp>
#include <cstring>

namespace gte::internal {

	void ParticleSystem::Render(const glm::mat4& parent)
	{
		for (const auto& particle : mParticlePool)
		{
			if (!particle.Active)
				continue;

			const float life = particle.RemaininigLife / particle.LifeTime;
			const glm::vec4 color = glm::lerp(particle.ColorEnd, particle.ColorBegin, life);
			const glm::vec2 size = glm::lerp(particle.ScaleEnd, particle.ScaleBegin, life);

			glm::mat4 transform = glm::translate(glm::mat4(1.0f), { particle.Position.x, particle.Position.y, 0.0f }) * glm::rotate(glm::mat4(1.0f), glm::radians(particle.Rotation), { 0.0f, 0.0f, 1.0f }) * glm::scale(glm::mat4(1.0f), { size.x, size.y, 1.0f });

			Renderer2D::DrawQuad(parent * transform, (uint32)entt::null, color);
		}
	}

	void ParticleSystem::Update(float dt)
	{
		mAccumulator += dt;
		if (!mProps.Looping && mAccumulator >= mProps.Duration)
			mActive = false;

		for (auto& particle : mParticlePool)
		{
			if (!particle.Active)
				continue;

			if (particle.RemaininigLife <= 0.0f)
			{
				particle.Active = false;
				continue;
			}

			particle.RemaininigLife -= dt;
			particle.Position += particle.Velocity * dt;
			particle.Rotation += particle.AngularVelocity * dt;
		}


		while (mAccumulator - mLastSpawn >= mProps.EmitionRate && mActive)
		{
			Emit();
			mLastSpawn += mProps.EmitionRate;
		}
		
	}

	void ParticleSystem::SetProps(const ParticleProps& props)
	{
		mProps = props;
		if (mParticlePool.size() < props.MaxParticles)
		{
			size_t toAdd = static_cast<size_t>(props.MaxParticles) - mParticlePool.size();
			const size_t size = mParticlePool.size() - mNextIndex - 1;
			const uint32 dstIndex = static_cast<uint32>(mNextIndex + toAdd);
			const uint32 srcIndex = mNextIndex + 1;
			if (size > 0)
				memcpy(&mParticlePool[dstIndex], &mParticlePool[srcIndex], size);
			for (uint32 i = mNextIndex + 1; i < toAdd; i++)
				mParticlePool[i].Active = false;
			mParticlePool.resize(props.MaxParticles);
			mNextIndex += static_cast<uint32>(toAdd);
		}
		else if(mParticlePool.size() > props.MaxParticles)
		{
			const size_t toRemove = mParticlePool.size() - static_cast<size_t>(props.MaxParticles);
			const size_t size = mParticlePool.size() - mNextIndex - 1;
			const int32 dstIndex = static_cast<uint32>(mNextIndex - toRemove);
			const uint32 srcIndex = mNextIndex + 1;
			if (dstIndex >= 0 && size > 0)
				memcpy(&mParticlePool[dstIndex], &mParticlePool[srcIndex], size);
			else if (size > 0)
				memcpy(&mParticlePool[0], &mParticlePool[srcIndex], size);
			mParticlePool.resize(props.MaxParticles);
			mNextIndex -= static_cast<uint32>(toRemove);
		}
	}

	void ParticleSystem::Emit(void)
	{
		Particle& particle = mParticlePool[mNextIndex];
		particle.Active = true;
		particle.Position = mProps.Position;
		particle.Rotation = mProps.Rotation;
		particle.Rotation += mProps.RotationVariation * (Random::Float() - 0.5f);

		particle.Velocity = mProps.Velocity;
		particle.Velocity.x += mProps.VelocityVariation.x * (Random::Float() - 0.5f);
		particle.Velocity.y += mProps.VelocityVariation.y * (Random::Float() - 0.5f);
		particle.AngularVelocity = mProps.AngularVelocity;
		particle.AngularVelocity += mProps.AngularVelocityVariation * (Random::Float() - 0.5f);

		particle.ColorBegin = mProps.ColorBegin;
		particle.ColorEnd = mProps.ColorEnd;
		
		particle.LifeTime = mProps.LifeTime;
		particle.RemaininigLife = mProps.LifeTime;

		particle.ScaleBegin = mProps.SizeBegin;
		particle.ScaleEnd = mProps.SizeEnd;

		mNextIndex--;
		if (mNextIndex < 0)
			mNextIndex = static_cast<uint32>(mParticlePool.size() - 1);
	}

	ParticleSystem::ParticleSystem(const ParticleProps& props)
	{
		mParticlePool.resize(mNextIndex + 1);
		SetProps(props);
	}

	void ParticleSystem::Start(void)
	{
		for (auto& particle : mParticlePool)
			particle.Active = false;

		mActive = true;
		mAccumulator = 0.0f;
		mLastSpawn = 0.0f;
	}

}