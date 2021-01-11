#ifndef _ENGINE_CORE
#define _ENGINE_CORE

//declspecs for Engine
#ifdef ENGINE_DLL
	#define ENGINE_API __declspec(dllexport)
#else
	#define ENGINE_API __declspec(dllimport)
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


#endif