#pragma once

#include <Engine/Core/Engine.h>
#include <Engine/Core/uuid.h>
#include <Engine/GPU/Texture.h>

//Forward Decleration
namespace YAML { class Node; }

namespace gte::internal {

	class ENGINE_API Animation {
	public:

		struct Frame {
			TextureCoordinates Coords;
			float Duration = 0.0f;
		};

	public:

		Animation(void) = default;
		Animation(const Animation& other);
		Animation(const YAML::Node& data);

		void Load(const YAML::Node& data);
		void Save(const uuid& id);

		void Update(float dt);

		void DeleteFrame(size_t index);


		[[nodiscard]] uuid& GetID(void) noexcept { return mID; }
		[[nodiscard]] const uuid& GetID(void) const { return mID; }

		[[nodiscard]] std::vector<Frame>& GetFrames(void) noexcept { return mFrames; }
		[[nodiscard]] const std::vector<Frame>& GetFrames(void) const noexcept { return mFrames; }

		[[nodiscard]] uuid& GetAtlas(void) noexcept { return mSpritesAtlas; }
		[[nodiscard]] const uuid& GetAtlas(void) const noexcept { return mSpritesAtlas; }

		[[nodiscard]] Frame& GetCurrentFrame(void) noexcept { return mFrames[mCurrentFrame]; }
		[[nodiscard]] const Frame& GetCurrentFrame(void) const noexcept { return mFrames[mCurrentFrame]; }

		[[nodiscard]] bool operator==(const Animation& rhs) const { return mID == rhs.mID; }
		[[nodiscard]] bool operator!=(const Animation& rhs) const { return mID != rhs.mID; }

		Animation& operator=(const Animation& rhs);

	private:

		uuid mID;
		uuid mSpritesAtlas;
		std::vector<Frame> mFrames;
		size_t mCurrentFrame = 0;
		float mAccumulator = 0.0f;
	};

}