#pragma once

#include "EngineCore.h"
#include <mutex>
#include <shared_mutex>

namespace GTE {

	template<typename T>
	class ENGINE_API Ref {

		struct PointerHandle {
			T* Pointer = nullptr;
			size_t RefCount = 0;
			std::mutex WMutex;
			std::shared_mutex RMutex;
		};

	public:

		Ref(void) { m_Handle = new PointerHandle(); }

		~Ref(void)
		{
			m_Handle->WMutex.lock();
			if (m_Handle->RefCount == 1)
			{
				m_Handle->WMutex.unlock();
				delete m_Handle->Pointer;
				delete m_Handle;
			}
			else
			{
				--(m_Handle->RefCount);
				m_Handle->WMutex.unlock();
			}
		}

		template<typename U>
		explicit Ref(U* ptr)
		{
			m_Handle = new PointerHandle();
			m_Handle->Pointer = ptr;
			m_Handle->RefCount = 1;
		}

		Ref(const Ref& other)
		{
			other.m_Handle->WMutex.lock();
			++(other.m_Handle->RefCount);
			m_Handle = other.m_Handle;
			other.m_Handle->WMutex.unlock();

		}

		Ref(Ref&& other)
		{
			other.m_Handle->WMutex.lock();
			m_Handle = other.m_Handle;
			other.m_Handle = new PointerHandle();
			m_Handle->WMutex.unlock();
		}

		template<typename U>
		Ref(const Ref<U>& other)
		{
			other.m_Handle->WMutex.lock();
			m_Handle = reinterpret_cast<PointerHandle*>(other.m_Handle);
			++(m_Handle->RefCount);
			other.m_Handle->WMutex.unlock();
		}

		template<typename U>
		Ref(Ref<U>&& other)
		{
			other.m_Handle->WMutex.lock();
			m_Handle = reinterpret_cast<PointerHandle*>(other.m_Handle);
			other.m_Handle = new Ref<U>::PointerHandle();
			m_Handle->WMutex.unlock();
		}

		Ref& operator=(const Ref& rhs)
		{
			if (this != &rhs)
			{
				m_Handle->WMutex.lock();
				if (m_Handle->RefCount == 1)
				{
					m_Handle->WMutex.unlock();
					delete m_Handle->Pointer;
					delete m_Handle;
				}
				else
				{
					--(m_Handle->RefCount);
					m_Handle->WMutex.unlock();
				}
				rhs.m_Handle->WMutex.lock();
				m_Handle = rhs.m_Handle;
				++(m_Handle->RefCount);
				rhs.m_Handle->WMutex.unlock();
			}
			return *this;
		}

		Ref& operator=(Ref&& rhs)
		{
			if (this != &rhs)
			{
				m_Handle->WMutex.lock();
				if (m_Handle->RefCount == 1)
				{
					m_Handle->WMutex.unlock();
					delete m_Handle->Pointer;
					delete m_Handle;
				}
				else
				{
					--(m_Handle->RefCount);
					m_Handle->WMutex.unlock();
				}
				rhs.m_Handle->WMutex.lock();
				m_Handle = rhs.m_Handle;
				rhs.m_Handle = new PointerHandle();
				m_Handle->WMutex.unlock();
			}
			return *this;
		}

		size_t Count(void) const
		{
			std::shared_lock ReadLock(m_Handle->RMutex);
			return m_Handle->RefCount;
		}

		T& operator*(void) { return *m_Handle->Pointer; }
		const T& operator*(void) const { return *m_Handle->Pointer; }

		T* operator->(void) { return Get(); }
		const T* operator->(void) const { return Get(); }

		T* Get(void) { return m_Handle->Pointer; }
		const T* Get(void) const { return m_Handle->Pointer; }

		bool Unique(void) const { return m_Handle->RefCount == 1; }

		explicit operator bool(void) const { return m_Handle->Pointer != nullptr; }

	private:

		PointerHandle* m_Handle = nullptr;

	private:

		template<typename U, typename ...Args>
		friend Ref<U> CreateRef(Args&& ...args);

		template<typename U>
		friend class Ref;

	};

	template<typename T, typename ...Args>
	Ref<T> CreateRef(Args&& ...args)
	{
		Ref<T> ref;
		ref.m_Handle->Pointer = new T(std::forward<Args>(args)...);
		ref.m_Handle->RefCount = 1;
		return ref;
	}


}