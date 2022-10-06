#pragma once
//TODO(Vasilis): Better change check for __clang__ to MACRO that will be define only in reflection

//declspecs for Engine
#ifdef __clang__
	#define ENGINE_API
#else
	#ifdef ENGINE_DLL
		#define ENGINE_API __declspec(dllexport)
	#else
		#define ENGINE_API __declspec(dllimport)
	#endif
#endif

//declspecs for game's logic
#ifdef GAME_DLL
	#define GAME_API __declspec(dllexport)
#else
	#define GAME_API __declspec(dllimport)
#endif

#ifdef PLATFORM_WINDOWS
	#define BREAK __debugbreak()
#elif PLATFORM_UNIX
	#include <signal.h>
	#define BREAK raise(SIGTRAP)
#endif

#ifdef DEBUG_BUILD
	#ifdef PLATFORM_WINDOWS
		#define DEBUG_BREAK __debugbreak()
	#elif PLATFORM_UNIX
		#define DEBUG_BREAK raise(SIGTRAP)
	#endif
	#define ENABLE_ASSERTS
#else
	#define DEBUG_BREAK
#endif

#ifdef ENABLE_ASSERTS
	#define ENGINE_ASSERT(x, ...) {if(!(x)){GTE_ERROR_LOG("Assertion failed: ", __VA_ARGS__); DEBUG_BREAK;}}
#else
	#define ENGINE_ASSERT(x, ...)
#endif

#define ASSERT(x, ...) {if(!(x)){GTE_ERROR_LOG("Assertion failed: ", __VA_ARGS__); BREAK;}}

#include <cstdint>

//Type definitions
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef float float32;
typedef double float64;

typedef unsigned char byte;

//Reflection macros
#ifdef __clang__
	#define ENUM(...) enum class __attribute__((annotate("enum:" #__VA_ARGS__)))
	#define COMPONENT(...) struct __attribute__((annotate("component:" #__VA_ARGS__)))
	#define SYSTEM(...) class __attribute__((annotate("system:" #__VA_ARGS__)))
	#define CLASS(...) class __attribute__((annotate("class:" #__VA_ARGS__)))
	#define PROPERTY(...) __attribute__((annotate("property:" #__VA_ARGS__)))
#elif defined(_WIN32)//TODO(Vasilis): #else when and if we change __clang__ to a MACRO
	#define ENUM(...) enum class GAME_API
	#define COMPONENT(...) struct GAME_API
	#define SYSTEM(...) class GAME_API
	#define CLASS(...) class GAME_API
	#define PROPERTY(...)
#endif

#include "Logger.h"