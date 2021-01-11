#ifndef _SCENE
#define _SCENE

#include "GreenTea/Core/EngineCore.h"

#include <sstream>

#include <glm.hpp>
#include <box2d/b2_world.h>

#pragma warning( push )
#pragma warning( disable : 4267 )
#include <entt.hpp>
#pragma warning( pop )
template<typename Type>
struct entt::type_index<Type> {};


//#include "Pools/TransformationPool.h"

namespace GTE {

	class ENGINE_API Entity;

	//TODO: Check sorting algorithm
	class ENGINE_API Scene {
	public:

		Scene(void);
		~Scene(void);

		Entity CreateEntity(const std::string& name=std::string());
		Entity GetEditorCamera(void);

		Entity GetSceneEntity(void);

		void Update(float dt);
		void Render(const glm::mat4* eyematrix);

		void Save(const char* filepath);
		void Load(const char* filepath);

		void TakeSnapshot(void);
		void ReloadSnapshot(void);

		void LoadGameLogic(const char* filepath = NULL);
		void UnloadGameLogic(void);

		void onViewportResize(uint32 width, uint32 height);

	private:

		void UpdateMatrices(void);

		void SetupWorld(void);
		void DestroyWorld(void);

		void FixedUpdate(void);

	private:

		float m_Accumulator = 0.0f;

		entt::registry m_Registry;
		entt::entity m_Me = entt::null;

		std::stringstream m_TempStorage;

		friend class Entity;
		friend class SceneManagerPanel;
		friend class CollisionDispatcher;
		friend ENGINE_API std::vector<Entity> GetEntities(const std::string&);
		//friend Entity RenderSceneManager(Scene& instance);
	};

	ENGINE_API std::vector<Entity> GetEntities(const std::string& Tag);

}

#endif