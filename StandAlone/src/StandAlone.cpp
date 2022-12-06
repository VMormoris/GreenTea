#include "StandAlone.h"

#include <gtc/matrix_transform.hpp>
#include <fstream>

static constexpr entt::entity EnttNull = entt::null;
static GPU::Texture* engineLogo = nullptr;
static bool ready = false;
static bool loaded = false;

StandAlone::StandAlone(void)
	: Application({ "Stand alone", 0, 0, 1080, 720, false, true, false, true })
{
#ifndef GT_WEB
	Image img("../Assets/Icons/GreenTeaLogo.png");
#else
	Image img("Assets/Icons/GreenTeaLogo.png");
#endif
	engineLogo = GPU::Texture2D::Create(img);

	Window* window = internal::GetContext()->GlobalWindow;
	internal::GetContext()->ScriptEngine = new gte::internal::ScriptingEngine();

	GPU::FrameBufferSpecification spec;
	spec.Attachments = { gte::GPU::TextureFormat::RGB8, gte::GPU::TextureFormat::UInt32 };
	spec.Width = window->GetWidth();
	spec.Height = window->GetHeight();
	internal::GetContext()->ViewportFBO = gte::GPU::FrameBuffer::Create(spec);
	internal::GetContext()->PixelBufferObject = gte::GPU::PixelBuffer::Create(spec.Width, spec.Height, gte::GPU::TextureFormat::UInt32);
	internal::GetContext()->PixelBufferObject->SetFramebuffer(gte::internal::GetContext()->ViewportFBO);

#ifndef GT_WEB
	std::filesystem::current_path("..");
#endif
	internal::GetContext()->AssetWatcher.LoadProject("./GameData");
	
	//Loading Script Assets
	bool loading = true;
	while (loading)
	{
		loading = false;
		const auto& assets = internal::GetContext()->AssetWatcher.GetAssets({ ".gtscript" });
		for (const auto& assetID : assets)
		{
			Ref<Asset> asset = internal::GetContext()->AssetManager.RequestAsset(assetID);
			if (asset->Type == AssetType::LOADING)
				loading = true;
		}
	}
	//Find First Scene
	std::string gtfile;
	for (const auto& entry : std::filesystem::directory_iterator("./GameData/"))
	{
		if (entry.path().extension() == ".gt")
			gtfile = entry.path().string();
	}
	std::string scenename;
	std::ifstream is(gtfile);
	std::getline(is, scenename);
	is.close();

	////Setup startup scene
	Scene* scene = new Scene();
	internal::GetContext()->ActiveScene = scene;
	internal::SceneSerializer serializer(scene);
	serializer.Deserialize("./GameData/Assets/" + scenename);
	scene->OnViewportResize(spec.Width, spec.Height);
	internal::GetContext()->ViewportSize = { static_cast<float>(spec.Width), static_cast<float>(spec.Height) };
}

void StandAlone::Update(float dt)
{
	constexpr float ClearColor = 30.0f / 255.0f;
	Scene* scene = internal::GetContext()->ActiveScene;
	GPU::FrameBuffer* viewportFBO = internal::GetContext()->ViewportFBO;

	//Update
	internal::GetContext()->GlobalTime += dt;
	Application::Update(dt);
	if(ready)
		scene->Update(dt);

	//Render
#ifndef GT_WEB
	viewportFBO->Bind();
#endif
	const auto& spec = viewportFBO->GetSpecification();
	RenderCommand::SetViewport(0, 0, spec.Width, spec.Height);
	RenderCommand::SetClearColor({ ClearColor, ClearColor, ClearColor, 1.0f });
	RenderCommand::Clear();
#ifndef GT_WEB
	viewportFBO->Clear(1, &EnttNull);
#endif
	if (ready)
	{
		Entity camera = scene->GetPrimaryCameraEntity();
		scene->Render(camera.GetComponent<CameraComponent>());
	}
	else
		RenderLogo();
#ifndef GT_WEB
	viewportFBO->Unbind();
	internal::GetContext()->PixelBufferObject->ReadPixels(1);
#endif

	mAccumulator += dt;
	if (mAccumulator >= 5.0f && loaded)
	{
		ready = true;
		internal::GetContext()->GlobalTime = 0.0f;
	}
	else if (!loaded)
	{
		const auto& assets = internal::GetContext()->AssetWatcher.GetAssets({ ".gtimg", ".gtaudio", ".gtfont", ".gtanimation" });
		bool loading = false;
		for (const auto& assetID : assets)
		{
			Ref<Asset> asset = internal::GetContext()->AssetManager.RequestAsset(assetID);
			if (asset->Type == AssetType::LOADING)
				loading = true;
		}
		if (!loading)
		{
			loaded = true;
			internal::GetContext()->Playing = true;
			internal::GetContext()->ActiveScene->OnStart();
		}
	}

#ifndef GT_WEB
	RenderCommand::DrawFramebuffer(viewportFBO);
#endif
}

StandAlone::~StandAlone(void) { internal::GetContext()->ActiveScene->OnStop(); }

void StandAlone::RenderLogo(void)
{
	Renderer2D::BeginScene(glm::mat4{ 1.0f });
	Renderer2D::DrawQuad(glm::scale(glm::mat4(1.0f), { 0.33f, 0.33f, 1.0f }), engineLogo, (uint32)EnttNull);
	Renderer2D::EndScene();
}