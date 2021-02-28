#pragma once

#include "EngineCore.h"
#include <memory>

namespace GTE {

	//My Ref implementation its bugged so 
	// I'm using shared_ptr
	template<typename T>
	using Ref = std::shared_ptr<T>;

	template<typename T, typename ...Args>
	constexpr Ref<T> CreateRef(Args&& ...args)
	{
		return std::make_shared<T>(std::forward<Args>(args)...);
	}


    /*template<typename T>
    class ENGINE_API Ref {

        struct PointerHandle {
            T* Pointer = nullptr;
            std::atomic<size_t> Counter{ 0 };
        };

    public:

        Ref(void) noexcept { m_Handle = new PointerHandle(); }
        ~Ref(void) noexcept
        {
            if (m_Handle->Counter.fetch_sub(1) == 1)
            {
                delete m_Handle->Pointer;
                delete m_Handle;
            }
        }

        template<typename U>
        explicit Ref(U* ptr) noexcept
        {
            m_Handle = new PointerHandle;
            m_Handle->Pointer = ptr;
            m_Handle->Counter.store(1);
        }

        Ref(const Ref& other) noexcept
        {
            ++m_Handle->Counter;
            m_Handle = other.m_Handle;
        }

        template<typename U>
        Ref(const Ref<U>& other) noexcept
        {
            ++other.m_Handle->Counter;
            m_Handle = reinterpret_cast<PointerHandle*>(other.m_Handle);
        }

        Ref(Ref&& other) noexcept
        {
            m_Handle = other.m_Handle;
            other.m_Handle = new PointerHandle();
        }

        template<typename U>
        Ref(Ref<U>&& other) noexcept
        {
            m_Handle = reinterpret_cast<PointerHandle*>(other.m_Handle);
            other.m_Handle = new typename Ref<U>::PointerHandle();
        }

        Ref& operator=(const Ref& rhs) noexcept
        {
            if (this != &rhs)
            {
                if (m_Handle->Counter.fetch_sub(1) == 1)
                {
                    delete m_Handle->Pointer;
                    delete m_Handle;
                }

                ++rhs.m_Handle->Counter;
                m_Handle = rhs.m_Handle;
            }
            return *this;
        }

        template<typename U>
        Ref& operator=(const Ref<U>& rhs) noexcept
        {
            if (this != &rhs)
            {
                if (m_Handle->Counter.fetch_sub(1) == 1)
                {
                    delete m_Handle->Pointer;
                    delete m_Handle;
                }

                ++rhs.m_Handle->Counter;
                m_Handle = reinterpret_cast<PointerHandle*>(rhs.m_Handle);
            }
            return *this;
        }

        Ref& operator=(Ref&& rhs) noexcept
        {
            if (this != &rhs)
            {
                if (m_Handle->Counter.fetch_sub(1) == 1)
                {
                    delete m_Handle->Pointer;
                    delete m_Handle;
                }

                m_Handle = rhs.m_Handle;
                rhs.m_Handle = new PointerHandle();
            }
            return *this;
        }

        template<typename U>
        Ref& operator=(Ref<U>&& rhs) noexcept
        {
            if (this != &rhs)
            {
                if (m_Handle->Counter.fetch_sub(1) == 1)
                {
                    delete m_Handle->Pointer;
                    delete m_Handle;
                }

                m_Handle = reinterpret_cast<PointerHandle*>(rhs.m_Handle);
                rhs.m_Handle = new typename Ref<U>::PointerHandle();
            }
            return *this;
        }

        T& operator*(void) noexcept { return *Get(); }
        const T& operator*(void) const noexcept { return *Get(); }

        T* operator->(void) noexcept { return Get(); }
        const T* operator->(void) const noexcept { return Get(); }

        inline T* Get(void) noexcept { return m_Handle->Pointer; }
        inline const T* Get(void) const noexcept { return m_Handle->Pointer; }
        inline size_t Count(void) const noexcept { return m_Handle->Counter.load(); }

    private:

        PointerHandle* m_Handle = nullptr;

        template<typename U, typename ...Args>
        friend constexpr Ref<U> CreateRef(Args&& ...args);

        template<typename U>
        friend class Ref;

    };

    template<typename T, typename ...Args>
    constexpr Ref<T> CreateRef(Args&& ...args)
    {
        Ref<T> ref;
        ref.m_Handle->Pointer = new T(std::forward<Args>(args)...);
        ref.m_Handle->Counter.store(1);
        return ref;
    }
    */


}