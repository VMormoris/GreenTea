#pragma once
#include "Scene.h"

#include <Engine/Core/uuid.h>

namespace gte {

	/**
	* @brief Basically an identifier for Game Objects
	*/
	class ENGINE_API Entity {
	public:
		Entity(void) = default;
		Entity(entt::entity entity, Scene* owner)
			: mEntityHandle(entity), mOwner(owner) {}
		
		template<typename T>
		[[nodiscard]] T& GetComponent(void)
		{
			ASSERT(HasComponent<T>(), "Requested component doesn't exist.");
			return mOwner->mReg.get<T>(mEntityHandle);
		}
		
		template<typename T>
		[[nodiscard]] bool HasComponent(void) const { return mOwner->mReg.any_of<T>(mEntityHandle); }
		
		template<typename T, typename ...Args>
		decltype(auto) AddComponent(Args&& ...args)
		{
			ASSERT(!HasComponent<T>(), "Requested component already exist.");
			return mOwner->mReg.emplace<T, Args...>(mEntityHandle, std::forward<Args>(args)...);
		}

		template<typename T>
		void RemoveComponent(void)
		{
			ASSERT(HasComponent<T>(), "Requested component doesn't exist.");
			mOwner->mReg.remove<T>(mEntityHandle);
		}

		//TODO(Vasilis): Add [[nodiscard]]
		operator bool(void) const { return mEntityHandle != entt::null && mOwner != nullptr && mOwner->mReg.valid(mEntityHandle); }
		operator entt::entity(void) const { return mEntityHandle; }
		operator uint32(void) const { return (uint32)mEntityHandle; }

		[[nodiscard]] uuid GetID(void);// { return GetComponent<IDComponent>().ID; }
		[[nodiscard]] std::string GetName(void);// { return GetComponent<TagComponent>().Tag; }

		bool operator==(const Entity& rhs) { return (mEntityHandle == rhs.mEntityHandle) && (mOwner == rhs.mOwner); }
		bool operator!=(const Entity& rhs) { return (mEntityHandle != rhs.mEntityHandle) || (mOwner != rhs.mOwner); }

	private:
		Scene* mOwner = nullptr;
		entt::entity mEntityHandle = entt::null;
	};

}