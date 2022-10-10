#include "ContentBrowserPanel.h"

#include <Engine/Core/FileDialog.h>
#include <Engine/ImGui/ImGuiWidgets.h>

#include <AL/al.h>
#include <AL/alext.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsForkAwesome.h>
#include <fstream>
#include <sndfile.h>

static gte::GPU::Texture* sFolder = nullptr;
static gte::GPU::Texture* sScriptFile = nullptr;
static gte::GPU::Texture* sSceneFile = nullptr;
static gte::GPU::Texture* sAudioFile = nullptr;
static gte::GPU::Texture* sTransparentTexture = nullptr;

ContentBrowserPanel::ContentBrowserPanel(const std::string& directory)
	: mParent(directory), mCurrentPath("Assets")
{
	mHistory.emplace_back(directory);
	gte::Image img("../Assets/Icons/folder.png");
	sFolder = gte::GPU::Texture2D::Create(img);

	img.Load("../Assets/Icons/cpp.png");
	sScriptFile = gte::GPU::Texture2D::Create(img);

	img.Load("../Assets/Icons/scene.png");
	sSceneFile = gte::GPU::Texture2D::Create(img);

	img.Load("../Assets/Icons/audio.png");
	sAudioFile = gte::GPU::Texture2D::Create(img);

	sTransparentTexture = gte::GPU::Texture2D::Create(1, 1);
	uint32 TransparentTexture = 0x00000000;
	sTransparentTexture->SetData(&TransparentTexture, sizeof(uint32));
}

void ContentBrowserPanel::Draw(void)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];

	ImGui::SetCursorPosX(ImGui::GetContentRegionAvail().x - 42.0f);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
	if (ImGui::Button("Import"))
		ImGui::OpenPopup("Import##popup");
	ImGui::PopStyleVar();
	ImGui::SameLine();

	ImGui::SetCursorPosX(4.0f);
	gte::gui::DrawSearchbar("Asset Search", mFilter, 64);
	ImGui::SameLine();

	ImGui::PushFont(IconsFont);
	ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 2.0f);
	const bool disablePrev = mCurrentIndex == 0;
	if (disablePrev)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	if (ImGui::Button(ICON_FK_CHEVRON_LEFT, { 24.0f, 24.0f }))
		mCurrentPath = mHistory[--mCurrentIndex];
	if (disablePrev)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
	ImGui::SameLine();
	const bool disableNext = mCurrentIndex == mHistory.size() - 1;
	if(disableNext)
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
	}
	if (ImGui::Button(ICON_FK_CHEVRON_RIGHT, { 24.0f, 24.0f }))
		mCurrentPath = mHistory[++mCurrentIndex];
	if (disableNext)
	{
		ImGui::PopItemFlag();
		ImGui::PopStyleVar();
	}
	ImGui::PopFont();
	ImGui::PopStyleVar();
	ImGui::SameLine();

	if (gte::gui::DrawBreadcrumb("Content Broswer Breadcrumb", mCurrentPath))
	{
		mHistory.emplace_back(mCurrentPath);
		mCurrentIndex++;
		mSelected = "";
	}
	
	if (ImGui::BeginPopup("Import##popup"))
	{
		if (gte::gui::DrawMenuItem(ICON_FK_FILE_IMAGE_O, "Texture", "", "Texture"))
		{
			std::filesystem::path filepath = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "All Images(*.png, *jpg, *.jpeg)\0*.png;*.jpg;*.jpeg\0PNG file (*.png)\0*.png\0JPG file(*.jpg, *.jpeg)\0*.jpg; *.jpeg\0");
			if (!filepath.empty())
				CreateTextureAsset(filepath);
			ImGui::CloseCurrentPopup();
		}
		if (gte::gui::DrawMenuItem(ICON_FK_MUSIC, "Audio", "", "Texture"))
		{
			std::filesystem::path filepath = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "All Audio(*.wav, *ogg)\0*.wav;*.ogg\0WAV file (*.wav)\0*.wav\0ogg file(*.ogg)\0*.ogg\0");
			if (!filepath.empty())
				CreateAudioAsset(filepath);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	ImGui::BeginChild("Assets");
	
	constexpr float padding = 32.0f;
	constexpr float thumbnailSize = 64.0f;
	constexpr float cellSize = thumbnailSize + 2 * padding;
	const float panelWidth = ImGui::GetContentRegionAvail().x;
	const int colCalc = (int)(panelWidth / cellSize);
	const int columnCount = colCalc > 1 ? colCalc : 1;

	bool openPopup = false;
	if (ImGui::BeginPopupContextWindow())
	{
		if (gte::gui::DrawMenuItem(ICON_FK_FOLDER_O, "Create Folder", nullptr))
		{
			CreateFolder(mCurrentPath, "New Folder");
			mShouldFocus = true;
		}
		else if (ImGui::BeginMenu("Native Script"))
		{
			//if (ImGui::MenuItem("Component"))
			//{
			//
			//}
			//else if (ImGui::MenuItem("System"))
			//{
			//
			//}
			if (ImGui::MenuItem("Scriptable Entity"))
				openPopup = true;
			ImGui::EndMenu();
		}
		ImGui::EndPopup();
	}
	ImGui::Columns(columnCount, 0, false);

	if (openPopup)
		ImGui::OpenPopup("Create Scriptable Entity");
	if (ImGui::BeginPopupModal("Create Scriptable Entity", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
	{
		static std::string error;
		ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
		ImGui::Text("Name:");
		ImGui::SameLine();
		static std::string name;
		char buffer[64];
		memcpy(buffer, name.c_str(), name.size() + 1);
		if (ImGui::InputText("##name", buffer, 64))
			name = std::string(buffer);

		ImGui::Dummy({ 0.0f, 0.0f });
		ImGui::SameLine(0.0f, 72.0f);
		if (ImGui::Button("Ok", { 60.0f, 0.0f }))
		{
			bool err = false;
			if (name.empty())
			{
				err = true;
				error = "Please type a name for your script.";
			}
			else
			{
				const auto& ids = gte::internal::GetContext()->AssetWatcher.GetAssets({ ".gtscript" });
				for (const auto& id : ids)
				{
					auto asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
					if (((gte::internal::NativeScript*)asset->Data)->GetName().compare(name) == 0)
					{
						err = true;
						error = "Script with this name already exists.";
						break;
					}
				}
			}
			if (err)
				ImGui::OpenPopup("Error");
			else
			{
				CreateScript(name);
				name = "";
				ImGui::CloseCurrentPopup();
			}
		}
		ImGui::SameLine();
		if (ImGui::Button("Cancel"))
			ImGui::CloseCurrentPopup();

		if (ImGui::BeginPopupModal("Error", NULL, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove))
		{
			ImGui::Text(error.c_str());
			ImGui::Dummy({ 0.0f, 0.0f });
			ImGui::SameLine(0.0f, 254.0f);
			if (ImGui::Button("Ok", { 60.0f, 0.0f }))
				ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
		ImGui::PopStyleVar();
		ImGui::EndPopup();
	}

	for (const auto entry : std::filesystem::directory_iterator(mCurrentPath))
	{
		const auto name = entry.path().stem().string();
		if (!mFilter.empty())//Check if it doesn't match filter to skip
		{
			std::string temp = "";
			std::for_each(name.begin(), name.end(), [&](const char c) { temp += std::tolower(c); });
			if (temp.find(mFilter) == std::string::npos)
				continue;
		}
		const float textSize = ImGui::CalcTextSize(name.c_str()).x;
		const float offsetCalc = (cellSize - textSize) * 0.5f;
		const float offset = offsetCalc > 0.0f ? offsetCalc : 0.0f;

		auto& colors = ImGui::GetStyle().Colors;
		auto selectionColor = colors[ImGuiCol_ButtonHovered];
		selectionColor.w *= 0.5f;

		if (entry.path() != mSelected)
			ImGui::PushStyleColor(ImGuiCol_Button, { 0.0f, 0.0f, 0.0f, 0.0f });
		else
			ImGui::PushStyleColor(ImGuiCol_Button, selectionColor);

		ImGui::PushID(entry.path().filename().string().c_str());
		if (!entry.is_directory())
		{
			const auto extension = entry.path().extension();
			if (extension == ".gtscene")
				ImGui::ImageButton(sSceneFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, static_cast<int32>(padding));
			else if (extension == ".gtscript" || extension == ".gtcomp" || extension == ".gtsystem")
				ImGui::ImageButton(sScriptFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, static_cast<int32>(padding));
			else if (extension == ".gtaudio")
				ImGui::ImageButton(sAudioFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, static_cast<int32>(padding));
			else if (extension == ".gtimg")
			{
				gte::uuid id = gte::internal::GetContext()->AssetWatcher.GetID(entry.path().string());
				gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
				if (asset->Type != gte::AssetType::TEXTURE)
					ImGui::ImageButton(sTransparentTexture->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, static_cast<int32>(padding));
				else
				{
					gte::GPU::Texture* texture = (gte::GPU::Texture*)asset->Data;
					ImGui::ImageButton(texture->GetID(), { 120.0f, 120.0f }, { 0, 1 }, { 1, 0 }, 4);
				}
			}
		}
		else
		{
			ImGui::ImageButton(sFolder->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, static_cast<int32>(padding));
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path filepath = (const char*)payload->Data;
					std::filesystem::rename(filepath, entry.path() / filepath.filename());
					gte::internal::GetContext()->AssetWatcher.FindFiles();
				}
			}
		}

		if (ImGui::BeginDragDropSource())
		{
			std::string path = entry.path().string();
			ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", path.c_str(), path.size() + 1);
			ImGui::Text(name.c_str());
			ImGui::EndDragDropSource();
		}
		ImGui::PopID();
		ImGui::PopStyleColor();
		if (ImGui::BeginPopupContextItem())
		{
			if (gte::gui::DrawMenuItem(ICON_FK_TRASH, "Delete", "Delete", "Delete Delete"))
				std::filesystem::remove_all(entry);
			if (ImGui::MenuItem("Rename"))
			{
				mRenaming = entry.path();
				mTempName = mRenaming.stem().string();
				mShouldFocus = true;
			}
			ImGui::EndPopup();
		}
		else if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
		{
			if (entry.is_directory())
			{
				mCurrentPath /= entry.path().filename();

				//Change history queue
				const size_t end = mHistory.size() - 1;
				for (size_t i = end; i > mCurrentIndex; i--)
					mHistory.emplace_back(mHistory[i]);
				mHistory.emplace_back(mHistory[mCurrentIndex]);
				for (size_t i = end; i > mCurrentIndex; i--)
					mHistory.erase(mHistory.begin() + i);
				mHistory.erase(mHistory.begin() + mCurrentIndex);
				mHistory.emplace_back(mCurrentPath);

				mCurrentIndex = mHistory.size() - 1;
			}
			else if(entry.path().extension() == ".gtscript")
			{
				gte::uuid id = gte::internal::GetContext()->AssetWatcher.GetID(entry.path().string());
				auto asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
				const auto& header = ((gte::internal::NativeScript*)asset->Data)->GetHeader();
				GTE_TRACE_LOG("Opening file: ", header, " on Visual Studio");
				std::string command = "start devenv /Edit \"" + header + "\"";
				system(command.c_str());
			}
			mSelected = "";
		}
		else if (ImGui::IsItemClicked())
			mSelected = entry.path();
		else if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && ImGui::IsWindowHovered())
			mSelected = "";

		if (!mRenaming.empty() && mRenaming == entry.path())
		{
			char buffer[64];
			memcpy(buffer, mTempName.c_str(), mTempName.size() + 1);
			if (mShouldFocus)
			{
				ImGui::SetKeyboardFocusHere();
				mShouldFocus = false;
			}
			ImGui::PushItemWidth(cellSize);
			if (ImGui::InputText("##Renaming", buffer, 64, ImGuiInputTextFlags_EnterReturnsTrue))
			{
				mTempName = std::string(buffer);
				if (!mTempName.empty()) {
					const auto out = mCurrentPath / mTempName;
					const auto filepath = out.string() + entry.path().extension().string();
					if(!std::filesystem::exists(filepath))
						std::filesystem::rename(entry, filepath);
				}
				Clear();
			}
			ImGui::PopItemWidth();
			if ((ImGui::IsMouseClicked(ImGuiMouseButton_Left) || ImGui::IsMouseClicked(ImGuiMouseButton_Right)) && (!mShouldFocus && !ImGui::IsItemHovered()))
				Clear();
		}
		else
		{
			ImGui::Dummy({ 0.0f, 0.0f });
			ImGui::SameLine(offset);
			ImGui::TextWrapped(name.c_str());
			if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
			{
				mRenaming = entry.path();
				mTempName = mRenaming.stem().string();
				mShouldFocus = true;
			}
		}
		ImGui::NextColumn();
	}
	ImGui::Columns(1);
	ImGui::EndChild();
}

void ContentBrowserPanel::Clear(void)
{
	mSelected = "";
	mRenaming = "";
	mShouldFocus = false;
}

void ContentBrowserPanel::CreateFolder(const std::filesystem::path& parent, std::string _template)
{
	auto path = parent / _template;
	if (std::filesystem::exists(path))
	{
		size_t i = 1;
		do
		{
			const auto name = _template + " (" + std::to_string(i) + ')';
			path = parent / name;
			i++;
		}
		while (std::filesystem::exists(path));
	}
	std::filesystem::create_directory(path);
	mRenaming = path;
	mTempName = mRenaming.stem().string();
}

void ContentBrowserPanel::CreateScript(const std::string& name) const
{
	std::time_t result = std::time(nullptr);
	auto filepath = mCurrentPath / (name + ".gtscript");	
	auto prjname = std::filesystem::current_path().filename().string();
	
	std::string data = "Name: " + name + "\n" + 
		"Header: "+ prjname + "\\src\\" + name + ".h\n" +
		"Version: 1\n" +
		"Type: 4\n" +
		"Fields:\n";

	std::ofstream os(filepath, std::ios::binary);
	os << "# Native-Script Asset for GreenTea //Engine\n";
	os << "# Auto generated by CupOfTea.exe at" << std::asctime(std::localtime(&result));
	os << "4\n";
	os << gte::uuid::Create() << '\n';
	os << data.size() << "\n\n";
	os << data;
	os.close();
	
	os.open(prjname + "/src/" + name + ".h");
	os << "#pragma once\n\n";
	os << "#include <GreenTea.h>\n\n";
	os << "using namespace gte;\n\n";
	os << "CLASS()\n";
	os << name << " : public ScriptableEntity {\n";
	os << "public:\n";
	os << '\t' << name << "() = default;\n";
	os << "};";
	os.close();

	os.open(prjname + "/src/" + name + ".cpp");
	os << "#include \""<< name << ".h\"\n\n";
	os.close();

	std::string command = "premake5 vs2019";
	system(command.c_str());

	command = "start devenv /Edit \"" + prjname + "\\src\\" + name + ".h\"";
	system(command.c_str());
}

void ContentBrowserPanel::DeleteSelected(void)
{
	if (mSelected == "")
		return;
	std::filesystem::remove_all(mSelected);
	mSelected = "";
}

void ContentBrowserPanel::CreateTextureAsset(const std::filesystem::path& filepath) const
{
	gte::Image img(filepath.string().c_str());
	size_t size = img.Size() + 12;
	auto path = (mCurrentPath / filepath.stem()).string() + ".gtimg";
	size_t i = 0;
	while (std::filesystem::exists(path))
		path = (mCurrentPath / filepath.stem()).string() + "(" + std::to_string(i++) + ").gtimg";
	std::time_t result = std::time(nullptr);
	std::ofstream os(path, std::ios::binary);
	os << "# Image for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 2 << '\n' << gte::uuid::Create() << '\n' << size << "\n\n";
	uint32 width = img.GetWidth();
	uint32 height = img.GetHeight();
	int32 bpp = img.GetBytePerPixel();
	os.write((char*)&width, sizeof(uint32));
	os.write((char*)&height, sizeof(uint32));
	os.write((char*)&bpp, sizeof(int32));
	os.write((char*)img.Data(), img.Size());
	os.close();
}

void ContentBrowserPanel::CreateAudioAsset(const std::filesystem::path& filepath) const
{
	SF_INFO info;
	SNDFILE* snd = sf_open(filepath.string().c_str(), SFM_READ, &info);
	ASSERT(snd, "Couldn't load file: ", filepath.string());

	ALenum format = AL_INVALID_ENUM;
	if (info.channels == 1)
		format = AL_FORMAT_MONO16;
	else if (info.channels == 2)
		format = AL_FORMAT_STEREO16;
	else if (info.channels == 3)
	{
		if (sf_command(snd, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
			format = AL_FORMAT_BFORMAT2D_16;
	}
	else if (info.channels == 4)
	{
		if (sf_command(snd, SFC_WAVEX_GET_AMBISONIC, NULL, 0) == SF_AMBISONIC_B_FORMAT)
			format = AL_FORMAT_BFORMAT3D_16;
	}
	
	if (format == AL_INVALID_ENUM)
	{
		sf_close(snd);
		GTE_ERROR_LOG("The file: ", filepath.string(), " doesn't use a supported format.");
		return;
	}

	size_t datasize = (info.frames * info.channels) * sizeof(int16);
	int16* data = new int16[info.frames * info.channels];
	size_t frames = sf_readf_short(snd, data, info.frames);
	if (frames == 0)
	{
		delete[] data;
		sf_close(snd);
		GTE_ERROR_LOG("Failed to read sample from file: ", filepath.string());
		return;
	}
	sf_close(snd);

	size_t size = datasize + 8;
	auto path = (mCurrentPath / filepath.stem()).string() + ".gtaudio";
	size_t i = 0;
	while (std::filesystem::exists(path))
		path = (mCurrentPath / filepath.stem()).string() + "(" + std::to_string(i++) + ").gtaudio";
	std::time_t result = std::time(nullptr);
	std::ofstream os(path, std::ios::binary);
	os << "# Audio for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 8 << '\n' << gte::uuid::Create() << '\n' << size << "\n\n";

	os.write((char*)&format, sizeof(ALenum));
	os.write((char*)&info.samplerate, sizeof(int32));
	os.write((char*)data, datasize);
	os.close();
	delete[] data;
}