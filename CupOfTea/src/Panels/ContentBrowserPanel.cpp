#include "ContentBrowserPanel.h"

#include <Engine/Core/FileDialog.h>
#include <Engine/ImGui/ImGuiWidgets.h>
#include <Engine/Assets/Font.h>

#include <AL/al.h>
#include <AL/alext.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <IconsForkAwesome.h>
#include <fstream>
#include <sndfile.h>
#include <msdf-atlas-gen/msdf-atlas-gen.h>

static gte::GPU::Texture* sFolder = nullptr;
static gte::GPU::Texture* sScriptFile = nullptr;
static gte::GPU::Texture* sSceneFile = nullptr;
static gte::GPU::Texture* sAudioFile = nullptr;
static gte::GPU::Texture* sPrefabFile = nullptr;
static gte::GPU::Texture* sFontFile = nullptr;
static gte::GPU::Texture* sAnimationFile = nullptr;
static gte::GPU::Texture* sTransparentTexture = nullptr;

static void SerializeEntity(gte::Entity entity, YAML::Emitter& out, bool recursive = false);
static void UpdatePrefab(gte::Entity entity, const std::filesystem::path& filepath);

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

	img.Load("../Assets/Icons/prefab.png");
	sPrefabFile = gte::GPU::Texture2D::Create(img);

	img.Load("../Assets/Icons/font.png");
	sFontFile = gte::GPU::Texture2D::Create(img);

	img.Load("../Assets/Icons/animation.png");
	sAnimationFile = gte::GPU::Texture2D::Create(img);

	sTransparentTexture = gte::GPU::Texture2D::Create(1, 1);
	uint32 TransparentTexture = 0x00000000;
	sTransparentTexture->SetData(&TransparentTexture, sizeof(uint32));
}

void ContentBrowserPanel::Draw(void)
{
	ImGuiIO& io = ImGui::GetIO();
	auto IconsFont = io.Fonts->Fonts[3];
	mAnimation = {};

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
		if (gte::gui::DrawMenuItem(ICON_FK_FILE_IMAGE_O, "Texture", nullptr, "Texture"))
		{
			std::filesystem::path filepath = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "All Images(*.png, *jpg, *.jpeg)\0*.png;*.jpg;*.jpeg\0PNG file (*.png)\0*.png\0JPG file(*.jpg, *.jpeg)\0*.jpg; *.jpeg\0");
			if (!filepath.empty())
				CreateTextureAsset(filepath);
			ImGui::CloseCurrentPopup();
		}
		if (gte::gui::DrawMenuItem(ICON_FK_MUSIC, "Audio", nullptr, "Texture"))
		{
			std::filesystem::path filepath = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "All Audio(*.wav, *ogg)\0*.wav;*.ogg\0WAV file (*.wav)\0*.wav\0ogg file(*.ogg)\0*.ogg\0");
			if (!filepath.empty())
				CreateAudioAsset(filepath);
			ImGui::CloseCurrentPopup();
		}
		if (gte::gui::DrawMenuItem(ICON_FK_FONT, "Font", nullptr, "Texture"))
		{
			std::filesystem::path filepath = gte::internal::CreateFileDialog(gte::internal::FileDialogType::Open, "All Fonts(*.ttf, *otf)\0*.ttf;*.otf\0True Type Font file(*.ttf)\0*.ttf\0Open Type Font file(*.otf)\0*.otf\0");
			if (!filepath.empty())
				CreateFontAsset(filepath);
			ImGui::CloseCurrentPopup();
		}
		ImGui::EndPopup();
	}
	ImGui::Separator();

	ImGui::BeginChild("Assets");
	constexpr int32 padding = 32;
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
			CreateFolder("New Folder");
			mShouldFocus = true;
		}
		else if (gte::gui::DrawMenuItem(ICON_FK_FILM, "Create Animation", nullptr))
		{
			CreateAnimation("New Animation");
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
				ImGui::ImageButton(sSceneFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
			else if (extension == ".gtscript" || extension == ".gtcomp" || extension == ".gtsystem")
				ImGui::ImageButton(sScriptFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
			else if (extension == ".gtaudio")
				ImGui::ImageButton(sAudioFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
			else if (extension == ".gtfont")
				ImGui::ImageButton(sFontFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
			else if (extension == ".gtanimation")
				ImGui::ImageButton(sAnimationFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
			else if (extension == ".gtprefab")
			{
				ImGui::ImageButton(sPrefabFile->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
				if (ImGui::BeginDragDropTarget())
				{
					if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
					{
						gte::uuid id = *(gte::uuid*)payload->Data;
						gte::Entity entity = gte::internal::GetContext()->ActiveScene->FindEntityWithUUID(id);
						UpdatePrefab(entity, entry.path());
					}
					ImGui::EndDragDropTarget();
				}
			}
			else if (extension == ".gtimg")
			{
				gte::uuid id = gte::internal::GetContext()->AssetWatcher.GetID(entry.path().string());
				gte::Ref<gte::Asset> asset = gte::internal::GetContext()->AssetManager.RequestAsset(id);
				if (asset->Type != gte::AssetType::TEXTURE)
					ImGui::ImageButton(sTransparentTexture->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
				else
				{
					gte::GPU::Texture* texture = (gte::GPU::Texture*)asset->Data;
					ImGui::ImageButton(texture->GetID(), { 120.0f, 120.0f }, { 0, 1 }, { 1, 0 }, 2);
				}
			}
		}
		else
		{
			ImGui::ImageButton(sFolder->GetID(), { 64.0f, 64.0f }, { 0, 1 }, { 1, 0 }, padding);
			if (ImGui::BeginDragDropTarget())
			{
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
				{
					std::filesystem::path filepath = (const char*)payload->Data;
					std::filesystem::rename(filepath, entry.path() / filepath.filename());
					gte::internal::GetContext()->AssetWatcher.FindFiles();
				}
				ImGui::EndDragDropTarget();
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
			else if (entry.path().extension() == ".gtanimation")
			{
				gte::uuid id = gte::internal::GetContext()->AssetWatcher.GetID(entry.path().string());
				mAnimation = id;
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

	if (ImGui::BeginDragDropTarget())
	{
		if (const auto* payload = ImGui::AcceptDragDropPayload("ENTITY_ITEM"))
		{
			gte::uuid id = *(gte::uuid*)payload->Data;
			if (id.IsValid())
			{
				gte::Entity entity = gte::internal::GetContext()->ActiveScene->FindEntityWithUUID(id);
				CreatePrefab(entity, mCurrentPath);
			}
		}
		ImGui::EndDragDropTarget();
	}
}

void ContentBrowserPanel::Clear(void)
{
	mSelected = "";
	mRenaming = "";
	mShouldFocus = false;
}

void ContentBrowserPanel::CreateFolder(const std::string& _template)
{
	auto path = mCurrentPath / _template;
	if (std::filesystem::exists(path))
	{
		size_t i = 1;
		do
		{
			const auto name = _template + " (" + std::to_string(i) + ')';
			path = mCurrentPath / name;
			i++;
		}
		while (std::filesystem::exists(path));
	}
	std::filesystem::create_directory(path);
	mRenaming = path;
	mTempName = mRenaming.stem().string();
}

void ContentBrowserPanel::CreateAnimation(const std::string& _template)
{
	auto path = mCurrentPath / _template;
	if (std::filesystem::exists(path))
	{
		size_t i = 1;
		do
		{
			const auto name = _template + " (" + std::to_string(i) + ')';
			path = mCurrentPath / name;
			i++;
		} while (std::filesystem::exists(path));
	}

	path = path.string() + ".gtanimation";
	mRenaming = path;
	mTempName = mRenaming.stem().string();

	std::time_t result = std::time(nullptr);
	std::ofstream os(path, std::ios::binary);
	os << "# Animation Asset for GreenTea Engine\n";
	os << "# Auto generated by CupOfTea.exe at " << std::asctime(std::localtime(&result));
	os << "9\n";
	os << gte::uuid::Create() << '\n';
	os << 0 << "\n\n";
	os.close();
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
	os << "# Native-Script Asset for GreenTea Engine\n";
	os << "# Auto generated by CupOfTea.exe at " << std::asctime(std::localtime(&result));
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

void ContentBrowserPanel::CreateFontAsset(const std::filesystem::path& filepath) const
{
	using namespace msdf_atlas;
	// Initialize FreeType
	msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype();
	if (!ft)
	{
		GTE_ERROR_LOG("Couldn't initialize FreeType.");
		return;
	}
	// Load font from file
	msdfgen::FontHandle* font = msdfgen::loadFont(ft, filepath.string().c_str());
	if (!font)
	{
		GTE_ERROR_LOG("Couldn't load font: \"", filepath.string(), "\".");
		msdfgen::deinitializeFreetype(ft);
		return;
	}

	//Load geometries
	std::vector<GlyphGeometry> glyphs;
	FontGeometry fontGeometry(&glyphs);
	fontGeometry.loadCharset(font, 1.0, Charset::ASCII);
	//Close handles to font & freetype 
	msdfgen::destroyFont(font);
	msdfgen::deinitializeFreetype(ft);

	// Paint glyphs for msdf
	const double maxCornerAngle = 3.0;
	for (auto& glyph : glyphs)
		glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, maxCornerAngle, 0);

	// Pack glyphs on atlas
	TightAtlasPacker packer;
	packer.setDimensionsConstraint(TightAtlasPacker::DimensionsConstraint::SQUARE);
	packer.setMinimumScale(32.0);
	packer.setPixelRange(2.0);
	packer.setMiterLimit(1.0);
	packer.pack(glyphs.data(), static_cast<int32>(glyphs.size()));

	//Generate Image
	int32 width = 0, height = 0;
	packer.getDimensions(width, height);
	ImmediateAtlasGenerator<float, 3, &msdfGenerator, BitmapAtlasStorage<byte, 3>> generator(width, height);
	GeneratorAttributes attributes;
	generator.setAttributes(attributes);
	generator.setThreadCount(4);
	generator.generate(glyphs.data(), static_cast<int32>(glyphs.size()));


	//Parse glyphs metadata
	if (fontGeometry.getKerning().size() == 0)
	{
		GTE_WARN_LOG("The font you choosen doesn't have kern table.\n\tYou might want to consider another font or re-exporting with an external tool like: FontForge.");
	}

	const auto& bitmap = (msdfgen::BitmapConstRef<byte, 3>)generator.atlasStorage();
	gte::Image img;
	img.Load(bitmap.pixels, bitmap.width, bitmap.height, 3);

	std::map<uint32, uint32> indexToCodepoint;
	std::map<uint32, gte::internal::Character> characters;
	for (const auto& glyph : glyphs)
	{
		gte::internal::Character character;
		uint32 unicode = glyph.getCodepoint();
		indexToCodepoint[glyph.getIndex()] = unicode;
		double l, r, b, t;
		glyph.getQuadAtlasBounds(l, b, r, t);
		l /= bitmap.width; r /= bitmap.width;
		b /= bitmap.height; t /= bitmap.height;
		character.UV = { static_cast<float>(l), static_cast<float>(b), static_cast<float>(r), static_cast<float>(t) };

		glyph.getQuadPlaneBounds(l, b, r, t);
		character.Quad = { static_cast<float>(l), static_cast<float>(b), static_cast<float>(r), static_cast<float>(t) };

		character.Advance = static_cast<float>(glyph.getAdvance());
		characters.emplace(unicode, character);
	}

	std::map<std::pair<uint32, uint32>, float> kernings;
	for (const auto& [key, val] : fontGeometry.getKerning())
		kernings.emplace
		(
			std::make_pair(indexToCodepoint[key.first], indexToCodepoint[key.second]),
			static_cast<float>(val)
		);

	//Write metadata to yaml
	YAML::Emitter out;
	out << YAML::BeginMap;
	out << YAML::Key << "Glyphs" << YAML::Value;
	out << YAML::BeginSeq;
	for (const auto& [unicode, character] : characters)
	{
		using namespace gte::math;
		out << YAML::BeginMap;
		out << YAML::Key << "Character" << YAML::Value << unicode;
		out << YAML::Key << "Metadata";
		out << YAML::BeginMap;
		out << YAML::Key << "UV" << YAML::Value << character.UV;
		out << YAML::Key << "Quad" << YAML::Value << character.Quad;
		out << YAML::Key << "Advance" << YAML::Value << character.Advance;
		out << YAML::EndMap;
		out << YAML::EndMap;
	}
	out << YAML::EndSeq;

	out << YAML::Key << "Kernings" << YAML::Value;
	out << YAML::BeginSeq;
	for (const auto& [key, kerning] : kernings)
	{

		out << YAML::BeginMap;
		out << YAML::Key << "Pair" << YAML::Value;
		out << YAML::Flow << YAML::BeginSeq << key.first << key.second << YAML::EndSeq;
		out << YAML::Key << "Kerning" << YAML::Value << kerning;
		out << YAML::EndMap;
	}
	out << YAML::EndSeq;
	out << YAML::Key << "Atlas";
	out << YAML::BeginMap;
	out << YAML::Key << "Width" << YAML::Value << bitmap.width;
	out << YAML::Key << "Height" << YAML::Value << bitmap.height;
	out << YAML::Key << "Channels" << YAML::Value << 3;
	out << YAML::EndMap;

	out << YAML::EndMap;
	
	auto path = (mCurrentPath / filepath.stem()).string() + ".gtfont";
	size_t i = 0;
	while (std::filesystem::exists(path))
		path = (mCurrentPath / filepath.stem()).string() + "(" + std::to_string(i++) + ").gtfont";
	
	std::time_t result = std::time(nullptr);
	std::ofstream os(path, std::ios::binary);
	os << "# Font for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 6 << '\n' << gte::uuid::Create() << '\n' << out.size() << "\n\n";

	os << out.c_str() << '\n';
	os.write((char*)img.Data(), img.Size());
	os.close();
}

void CreatePrefab(gte::Entity entity, const std::filesystem::path& dir)
{
	YAML::Emitter out;
	out << YAML::BeginSeq;
	SerializeEntity(entity, out);
	out << YAML::EndSeq;

	const auto name = entity.GetName();
	auto path = (dir / name).string() + ".gtprefab";
	size_t i = 0;
	while (std::filesystem::exists(path))
		path = (dir / name).string() + "(" + std::to_string(i++) + ").gtprefab";
	std::time_t result = std::time(nullptr);
	std::ofstream os(path, std::ios::binary);
	os << "# Prefab for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 11 << '\n' << gte::uuid::Create() << '\n' << out.size() << "\n\n";

	os << out.c_str();
	os.close();
}

void UpdatePrefab(gte::Entity entity, const std::filesystem::path& filepath)
{
	YAML::Emitter out;
	out << YAML::BeginSeq;
	SerializeEntity(entity, out);
	out << YAML::EndSeq;

	gte::uuid id = gte::internal::GetContext()->AssetWatcher.GetID(filepath.string());
	std::time_t result = std::time(nullptr);
	std::ofstream os(filepath, std::ios::binary);
	os << "# Prefab for Green Tea Engine\n# Auto generated by Green Tea at " << std::asctime(std::localtime(&result)) << 11 << '\n' << id << '\n' << out.size() << "\n\n";
	os << out.c_str();
	os.close();
}

void SerializeEntity(gte::Entity entity, YAML::Emitter& out, bool recursive)
{
	using namespace gte;
	using namespace gte::math;

	Scene* scene = internal::GetContext()->ActiveScene;

	out << YAML::BeginMap;
	out << YAML::Key << "Entity" << YAML::Value << entity.GetID().str();
	out << YAML::Key << "TagComponent";
	out << YAML::BeginMap;
	out << YAML::Key << "Tag" << YAML::Value << entity.GetName();
	out << YAML::EndMap;

	out << YAML::Key << "RelationshipComponent";
	out << YAML::BeginMap;
	const auto& rc = entity.GetComponent<gte::RelationshipComponent>();
	out << YAML::Key << "Childrens" << YAML::Value << rc.Childrens;
	out << YAML::Key << "FirstChild" << YAML::Value << (rc.FirstChild == entt::null ? uuid().str() : Entity{ rc.FirstChild, scene }.GetID().str());
	out << YAML::Key << "Previous" << YAML::Value << (rc.Previous == entt::null ? uuid().str() : Entity{ rc.Previous, scene }.GetID().str());
	out << YAML::Key << "Next" << YAML::Value << (rc.Next == entt::null ? uuid().str() : Entity{ rc.Next, scene }.GetID().str());
	if(!recursive)
		out << YAML::Key << "Parent" << YAML::Value << uuid().str();
	else
		out << YAML::Key << "Parent" << YAML::Value << (rc.Parent == entt::null ? uuid().str() : Entity{ rc.Parent, scene }.GetID().str());
	out << YAML::EndMap;

	if (entity.HasComponent<Transform2DComponent>())
	{
		out << YAML::Key << "Transform2DComponent";
		out << YAML::BeginMap;
		const auto& tc = entity.GetComponent<Transform2DComponent>();
		out << YAML::Key << "Position" << YAML::Value << tc.Position;
		out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
		out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<SpriteRendererComponent>())
	{
		out << YAML::Key << "SpriteRendererComponent";
		out << YAML::BeginMap;

		const auto& sprite = entity.GetComponent<SpriteRendererComponent>();
		out << YAML::Key << "Color" << YAML::Value << sprite.Color;
		out << YAML::Key << "Visible" << YAML::Value << sprite.Visible;
		out << YAML::Key << "Texture" << YAML::Value << sprite.Texture->ID.str();
		if (sprite.Texture->ID.IsValid())
		{
			out << YAML::Key << "TilingFactor" << YAML::Value << sprite.TilingFactor;
			out << YAML::Key << "FlipX" << YAML::Value << sprite.FlipX;
			out << YAML::Key << "FlipY" << YAML::Value << sprite.FlipY;
			out << YAML::Key << "TextureCoordinates";
			out << YAML::BeginMap;
			out << YAML::Key << "BottomLeft" << YAML::Value << sprite.Coordinates.BottomLeft;
			out << YAML::Key << "BottomRight" << YAML::Value << sprite.Coordinates.BottomRight;
			out << YAML::Key << "TopRight" << YAML::Value << sprite.Coordinates.TopRight;
			out << YAML::Key << "TopLeft" << YAML::Value << sprite.Coordinates.TopLeft;
			out << YAML::EndMap;
		}
		out << YAML::EndMap;
	}

	if (entity.HasComponent<CircleRendererComponent>())
	{
		const auto& circle = entity.GetComponent<CircleRendererComponent>();
		out << YAML::Key << "CircleRendererComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Color" << YAML::Value << circle.Color;
		out << YAML::Key << "Thickness" << YAML::Value << circle.Thickness;
		out << YAML::Key << "Fade" << YAML::Value << circle.Fade;
		out << YAML::Key << "Visible" << YAML::Value << circle.Visible;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<TextRendererComponent>())
	{
		const auto& tc = entity.GetComponent<TextRendererComponent>();
		out << YAML::Key << "TextRendererComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Text" << YAML::Value << tc.Text;
		out << YAML::Key << "Color" << YAML::Value << tc.Color;
		out << YAML::Key << "Font" << YAML::Value << tc.Font->ID.str();
		out << YAML::Key << "Size" << YAML::Value << tc.Size;
		out << YAML::Key << "Visible" << YAML::Value << tc.Visible;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<CameraComponent>())
	{
		const auto& ortho = entity.GetComponent<OrthographicCameraComponent>();
		const auto& cam = entity.GetComponent<CameraComponent>();
		out << YAML::Key << "CameraComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "ZoomLevel" << YAML::Value << ortho.ZoomLevel;
		out << YAML::Key << "VerticalBoundary" << YAML::Value << ortho.VerticalBoundary;
		if (cam.FixedAspectRatio)
			out << YAML::Key << "AspectRatio" << YAML::Value << cam.AspectRatio;
		out << YAML::Key << "Primary" << YAML::Value << cam.Primary;
		out << YAML::Key << "FixedAspectRatio" << YAML::Value << cam.FixedAspectRatio;
		out << YAML::Key << "MasterVolume" << YAML::Value << cam.MasterVolume;
		out << YAML::Key << "DistanceModel" << YAML::Value << (uint16)cam.Model;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<Rigidbody2DComponent>())
	{
		out << YAML::Key << "Rigidbody2DComponent";
		out << YAML::BeginMap;

		const auto& rb = entity.GetComponent<Rigidbody2DComponent>();
		out << YAML::Key << "Type" << YAML::Value << (uint64)rb.Type;
		out << YAML::Key << "Velocity" << YAML::Value << rb.Velocity;
		out << YAML::Key << "AngularVelocity" << YAML::Value << rb.AngularVelocity;
		out << YAML::Key << "GravityFactor" << YAML::Value << rb.GravityFactor;
		out << YAML::Key << "FixedRotation" << YAML::Value << rb.FixedRotation;
		out << YAML::Key << "Bullet" << YAML::Value << rb.Bullet;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<BoxColliderComponent>())
	{
		out << YAML::Key << "BoxColliderComponent";
		out << YAML::BeginMap;

		const auto& bc = entity.GetComponent<BoxColliderComponent>();
		out << YAML::Key << "Offset" << YAML::Value << bc.Offset;
		out << YAML::Key << "Size" << YAML::Value << bc.Size;
		out << YAML::Key << "Density" << YAML::Value << bc.Density;
		out << YAML::Key << "Friction" << YAML::Value << bc.Friction;
		out << YAML::Key << "Restitution" << YAML::Value << bc.Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << bc.RestitutionThreshold;
		out << YAML::Key << "Sensor" << YAML::Value << bc.Sensor;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<CircleColliderComponent>())
	{
		out << YAML::Key << "CircleColliderComponent";
		out << YAML::BeginMap;

		const auto& cc = entity.GetComponent<CircleColliderComponent>();
		out << YAML::Key << "Offset" << YAML::Value << cc.Offset;
		out << YAML::Key << "Radius" << YAML::Value << cc.Radius;
		out << YAML::Key << "Density" << YAML::Value << cc.Density;
		out << YAML::Key << "Friction" << YAML::Value << cc.Friction;
		out << YAML::Key << "Restitution" << YAML::Value << cc.Restitution;
		out << YAML::Key << "RestitutionThreshold" << YAML::Value << cc.RestitutionThreshold;
		out << YAML::Key << "Sensor" << YAML::Value << cc.Sensor;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<NativeScriptComponent>())
	{
		out << YAML::Key << "NativeScriptComponent";
		out << YAML::BeginMap;

		const auto& nc = entity.GetComponent<NativeScriptComponent>();
		out << YAML::Key << "Asset" << YAML::Value << nc.ScriptAsset->ID.str();
		out << YAML::Key << "Version" << YAML::Value << nc.Description.GetVersion();
		out << YAML::Key << "Properties" << YAML::Value << YAML::BeginSeq;
		const void* buffer = nc.Description.GetBuffer();
		for (const auto& prop : nc.Description.GetFieldsSpecification())
		{
			out << YAML::BeginMap;
			out << YAML::Key << "Name" << YAML::Value << prop.Name;
			out << YAML::Key << "Type" << YAML::Value << (uint16)prop.Type;
			out << YAML::Key << "Default" << YAML::Value;
			const void* ptr = (const byte*)buffer + prop.BufferOffset;
			switch (prop.Type)
			{
				using namespace gte::internal;
			case FieldType::Bool:
				out << *(bool*)ptr;
				break;
			case FieldType::Char:
			case FieldType::Enum_Char:
				out << *(int16*)ptr;
				break;
			case FieldType::Int16:
			case FieldType::Enum_Int16:
				out << *(int16*)ptr;
				break;
			case FieldType::Int32:
			case FieldType::Enum_Int32:
				out << *(int32*)ptr;
				break;
			case FieldType::Int64:
			case FieldType::Enum_Int64:
				out << *(int64*)ptr;
				break;
			case FieldType::Byte:
			case FieldType::Enum_Byte:
				out << (uint16) * (byte*)ptr;
				break;
			case FieldType::Uint16:
			case FieldType::Enum_Uint16:
				out << *(uint16*)ptr;
				break;
			case FieldType::Uint32:
			case FieldType::Enum_Uint32:
				out << *(uint32*)ptr;
				break;
			case FieldType::Uint64:
			case FieldType::Enum_Uint64:
				out << *(uint64*)ptr;
				break;
			case FieldType::Float32:
				out << *(float*)ptr;
				break;
			case FieldType::Float64:
				out << *(double*)ptr;
				break;
			case FieldType::Vec2:
				out << *(glm::vec2*)ptr;
				break;
			case FieldType::Vec3:
				out << *(glm::vec3*)ptr;
				break;
			case FieldType::Vec4:
				out << *(glm::vec4*)ptr;
				break;
			case FieldType::String:
				out << *(std::string*)ptr;
				break;
			case FieldType::Asset:
				out << (*(Ref<Asset>*)ptr)->ID.str();
				break;
			case FieldType::Entity:
				out << ((Entity*)ptr)->GetID().str();
				break;
			}
			out << YAML::EndMap;
		}
		out << YAML::EndSeq;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<SpeakerComponent>())
	{
		const auto& speaker = entity.GetComponent<SpeakerComponent>();
		out << YAML::Key << "SpeakerComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "AudioClip" << YAML::Value << speaker.AudioClip->ID.str();
		out << YAML::Key << "Volume" << YAML::Value << speaker.Volume;
		out << YAML::Key << "Pitch" << YAML::Value << speaker.Pitch;
		out << YAML::Key << "RollOffFactor" << YAML::Value << speaker.RollOffFactor;
		out << YAML::Key << "RefDistance" << YAML::Value << speaker.RefDistance;
		out << YAML::Key << "MaxDistance" << YAML::Value << speaker.MaxDistance;
		out << YAML::Key << "Looping" << YAML::Value << speaker.Looping;
		out << YAML::Key << "PlayOnStart" << YAML::Value << speaker.PlayOnStart;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<ParticleSystemComponent>())
	{
		const auto& psc = entity.GetComponent<ParticleSystemComponent>();
		out << YAML::Key << "ParticleSystemComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Position" << YAML::Value << psc.Props.Position;
		out << YAML::Key << "Velocity" << YAML::Value << psc.Props.Velocity;
		out << YAML::Key << "VelocityVariation" << YAML::Value << psc.Props.VelocityVariation;
		out << YAML::Key << "ColorBegin" << YAML::Value << psc.Props.ColorBegin;
		out << YAML::Key << "ColorEnd" << YAML::Value << psc.Props.ColorEnd;
		out << YAML::Key << "SizeBegin" << YAML::Value << psc.Props.SizeBegin;
		out << YAML::Key << "SizeEnd" << YAML::Value << psc.Props.SizeEnd;
		out << YAML::Key << "Rotation" << YAML::Value << psc.Props.Rotation;
		out << YAML::Key << "AngularVelocity" << YAML::Value << psc.Props.AngularVelocity;
		out << YAML::Key << "AngularVelocityVariation" << YAML::Value << psc.Props.AngularVelocityVariation;
		out << YAML::Key << "Duration" << YAML::Value << psc.Props.Duration;
		out << YAML::Key << "LifeTime" << YAML::Value << psc.Props.LifeTime;
		out << YAML::Key << "EmitionRate" << YAML::Value << psc.Props.EmitionRate;
		out << YAML::Key << "MaxParticles" << YAML::Value << psc.Props.MaxParticles;
		out << YAML::Key << "Looping" << YAML::Value << psc.Props.Looping;
		out << YAML::Key << "PlayOnStart" << YAML::Value << psc.PlayOnStart;
		out << YAML::EndMap;
	}

	if (entity.HasComponent<AnimationComponent>())
	{
		const auto& ac = entity.GetComponent<AnimationComponent>();
		out << YAML::Key << "AnimationComponent";
		out << YAML::BeginMap;
		out << YAML::Key << "Animation" << YAML::Value << ac.Animation->ID.str();
		out << YAML::EndMap;
	}
	out << YAML::EndMap;

	Entity child = { rc.FirstChild, scene };
	for (size_t i = 0; i < rc.Childrens; i++)
	{
		SerializeEntity(child, out, true);
		auto nextID = child.GetComponent<RelationshipComponent>().Next;
		child = { nextID, scene };
	}
}