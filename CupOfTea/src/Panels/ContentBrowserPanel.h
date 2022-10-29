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

private:
	
	void CreateScript(const std::string& name) const;
	
	void CreateFolder(const std::string& _template);
	void CreateAnimation(const std::string& _template);

	void CreateTextureAsset(const std::filesystem::path& filepath) const;
	void CreateAudioAsset(const std::filesystem::path& filepath) const;
	void CreateFontAsset(const std::filesystem::path& filepath) const;

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

	bool mShouldFocus = false;
};

void CreatePrefab(gte::Entity entity, const std::filesystem::path& dir);

