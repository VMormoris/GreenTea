#pragma once

#include <GreenTea.h>

#include <filesystem>

class ContentBrowserPanel {
public:
	ContentBrowserPanel(void) = default;
	ContentBrowserPanel(const std::string& directory);

	void Draw(void);
	void Clear(void);

	void DeleteSelected(void);

	const std::filesystem::path& GetCurrentPath(void) const { return mCurrentPath; }
	const gte::uuid& GetAnimation(void) const { return mAnimation; }
	const gte::uuid& GetMaterial(void) const { return mMaterial; }

	void PasteTo(const std::filesystem::path& source, const std::filesystem::path& target = "");

private:
	
	gte::uuid CreateScript(const std::string& name, gte::internal::ReflectionType scriptType) const;
	
	void CreateFolder(const std::string& _template);
	gte::uuid CreateAnimation(const std::string& _template);

	gte::uuid CreateTextureAsset(const std::filesystem::path& filepath) const;
	gte::uuid CreateAudioAsset(const std::filesystem::path& filepath) const;
	gte::uuid CreateFontAsset(const std::filesystem::path& filepath) const;
	gte::uuid CreateMeshAsset(const std::filesystem::path& filepath) const;

private:

	std::vector<std::filesystem::path> mHistory;
	size_t mCurrentIndex = 0;
	std::filesystem::path mParent;
	std::filesystem::path mCurrentPath;
	std::filesystem::path mRenaming;
	std::filesystem::path mSelected;
	std::string mTempName;
	std::string mFilter;
	gte::uuid mAnimation;
	gte::uuid mMaterial;

	bool mShouldFocus = false;
};

void CreatePrefab(gte::Entity entity, const std::filesystem::path& dir);

