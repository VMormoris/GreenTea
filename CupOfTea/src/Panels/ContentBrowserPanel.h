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

private:
	void CreateScript(const std::string& name);
	void CreateFolder(const std::filesystem::path& parent, std::string _template);

private:
	std::vector<std::filesystem::path> mHistory;
	size_t mCurrentIndex = 0;
	std::filesystem::path mParent;
	std::filesystem::path mCurrentPath;
	std::filesystem::path mRenaming;
	std::filesystem::path mSelected;
	std::string mTempName;
	std::string mFilter;

	bool mShouldFocus = false;
};

