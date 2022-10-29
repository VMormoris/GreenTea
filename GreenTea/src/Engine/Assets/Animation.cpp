#include "Animation.h"

#include <Engine/Core/Context.h>
#include <Engine/Core/Math.h>

#include <yaml-cpp/yaml.h>
#include <fstream>

namespace gte::internal {

	void Animation::Load(const YAML::Node& data)
	{
		using namespace gte::math;
		mFrames.clear();
		mAccumulator = 0.0f;
		mCurrentFrame = 0;

		mSpritesAtlas = data["SpritesAtlas"].as<std::string>();
		const auto& frames = data["Frames"];
		mFrames.reserve(frames.size());
		for (const auto& frame : frames)
		{
			Frame fr;
			fr.Coords.BottomLeft = frame["BottomLeft"].as<glm::vec2>();
			fr.Coords.BottomRight = frame["BottomRight"].as<glm::vec2>();
			fr.Coords.TopRight = frame["TopRight"].as<glm::vec2>();
			fr.Coords.TopLeft = frame["TopLeft"].as<glm::vec2>();
			fr.Duration = frame["Duration"].as<float>();

			mFrames.push_back(fr);
		}
	}

	void Animation::Save(const uuid& id)
	{
		using namespace gte::math;
		std::string filepath = internal::GetContext()->AssetWatcher.GetFilepath(id);

		YAML::Emitter out;
		out << YAML::BeginMap;
		out << YAML::Key << "SpritesAtlas" << YAML::Value << mSpritesAtlas.str();
		out << YAML::Key << "Frames" << YAML::Value;
		out << YAML::BeginSeq;
		for (const auto& frame : mFrames)
		{
			out << YAML::BeginMap;
			out << YAML::Key << "BottomLeft" << YAML::Value << frame.Coords.BottomLeft;
			out << YAML::Key << "BottomRight" << YAML::Value << frame.Coords.BottomRight;
			out << YAML::Key << "TopRight" << YAML::Value << frame.Coords.TopRight;
			out << YAML::Key << "TopLeft" << YAML::Value << frame.Coords.TopLeft;
			out << YAML::Key << "Duration" << YAML::Value << frame.Duration;
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;

		std::time_t result = std::time(nullptr);
		std::ofstream os(filepath, std::ios::binary);
		os << "# Animation Asset for GreenTea Engine\n";
		os << "# Auto generated by CupOfTea.exe at " << std::asctime(std::localtime(&result));
		os << "9\n";
		os << id.str() << '\n';
		os << out.size() << "\n\n";

		os << out.c_str();
		os.close();
	}

	void Animation::Update(float dt)
	{
		mAccumulator += dt;
		const Frame& frame = mFrames[mCurrentFrame];
		if (mAccumulator >= frame.Duration)
		{
			mCurrentFrame = (mCurrentFrame + 1) % mFrames.size();
			mAccumulator -= frame.Duration;
		}
	}

	void Animation::DeleteFrame(size_t index)
	{
		mFrames.erase(mFrames.begin() + index);
		if (index == mCurrentFrame)
			mCurrentFrame = (mCurrentFrame + 1) % mFrames.size();
	}

	Animation::Animation(const Animation& other)
	{
		mID = other.mID;
		mSpritesAtlas = other.mSpritesAtlas;
		mFrames = other.mFrames;

		mCurrentFrame = 0;
		mAccumulator = 0.0f;
	}

	Animation& Animation::operator=(const Animation& rhs)
	{
		if (this != &rhs)
		{
			mID = rhs.mID;
			mSpritesAtlas = rhs.mSpritesAtlas;
			mFrames = rhs.mFrames;

			mCurrentFrame = 0;
			mAccumulator = 0.0f;
		}
		return *this;
	}

	Animation::Animation(const YAML::Node& data) { Load(data); }

}