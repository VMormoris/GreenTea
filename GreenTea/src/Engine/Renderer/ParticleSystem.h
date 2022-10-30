#pragma once

#include <Engine/Core/Engine.h>

#include <glm.hpp>
namespace gte::internal {

	struct ENGINE_API ParticleProps {

		glm::vec2 Position = { 0.0f, 0.0f };
		glm::vec2 Velocity = { 0.0f, 0.0f }, VelocityVariation = { 0.0f, 0.0f };
		glm::vec4 ColorBegin = { 1.0f, 1.0f, 1.0f, 1.0f }, ColorEnd = { 1.0f, 1.0f, 1.0f, 1.0f };
		glm::vec2 SizeBegin = { 1.0f, 1.0f }, SizeEnd = { 1.0f, 1.0f };
		float Rotation = 0.0f, RotationVariation = 0.0f;
		float AngularVelocity = 0.0f, AngularVelocityVariation = 0.0f;
		float Duration = 5.0f;
		float LifeTime = 1.0f;
		float EmitionRate = 10.0f / 1000.0f;
		uint32 MaxParticles = 1000;
		bool Looping = false;
	};

	class ENGINE_API ParticleSystem {
	public:
		ParticleSystem(const ParticleProps& props);

		void Start(void);
		
		void Update(float dt);
		void Render(const glm::mat4& parent);

		void SetProps(const ParticleProps& props);
		ParticleProps& GetProps(void) noexcept { return mProps; }
		const ParticleProps& GetProps(void) const noexcept { return mProps; }

	private:

		void Emit(void);
	
	private:
		struct Particle {
			glm::vec2 Position = { 0.0f, 0.0f };
			glm::vec2 Velocity = { 0.0f, 0.0f };
			glm::vec4 ColorBegin = { 1.0f, 1.0f, 1.0f, 1.0f }, ColorEnd = { 1.0f, 1.0f, 1.0f, 1.0f };
			
			float Rotation = 0.0f;
			float AngularVelocity = 0.01f;
			
			glm::vec2 ScaleBegin = { 1.0f, 1.0f }, ScaleEnd = { 1.0f, 1.0f };

			float LifeTime = 1.0f;
			float RemaininigLife = 0.0f;

			bool Active = false;
		};
	private:

		std::vector<Particle> mParticlePool;
		int32 mNextIndex = 999;
		ParticleProps mProps;
		bool mActive = false;
		float mAccumulator = 0.0f;
		float mLastSpawn = 0.0f;
	};

}