#pragma once

//declspecs for Engine & Games' logic
#ifdef PLATFORM_WINDOWS
	#ifdef ENGINE_DLL
		#define ENGINE_API __declspec(dllexport)
	#else
		#define ENGINE_API __declspec(dllimport)
	#endif

	#ifdef GAME_DLL
		#define GAME_API __declspec(dllexport)
	#else
		#define GAME_API __declspec(dllimport)
	#endif
#else
	#define ENGINE_API
	#define GAME_API
#endif

#ifdef PLATFORM_WINDOWS
	#define BREAK __debugbreak()
#elif PLATFORM_UNIX
	#include <signal.h>
	#define BREAK raise(SIGTRAP)
#endif

#ifdef DEBUG_BUILD
	#define DEBUG_BREAK BREAK
	#define ENABLE_ASSERTS
#else
	#define DEBUG_BREAK
#endif

#ifdef ENABLE_ASSERTS
	#define ENGINE_ASSERT(x, ...) {if(!(x)){GTE_ERROR_LOG("Assertion failed: ", __VA_ARGS__); DEBUG_BREAK;}}
#else
	#define ENGINE_ASSERT(x, ...)
#endif


#define ASSERT(x, ...) {if(!(x)){GTE_ERROR_LOG("Assertion failed: ", __VA_ARGS__); throw gte::AssertException();}}

#include <cstdint>

//Type definitions
typedef std::int8_t int8;
typedef std::int16_t int16;
typedef std::int32_t int32;
typedef std::int64_t int64;

typedef std::uint8_t uint8;
typedef std::uint16_t uint16;
typedef std::uint32_t uint32;
typedef std::uint64_t uint64;

typedef float float32;
typedef double float64;

typedef unsigned char byte;

//Rust-like types
using i8 = int8;
using i16 = int16;
using i32 = int32;
using i64 = int64;

using u8 = uint8;
using u16 = uint16;
using u32 = uint32;
using u64 = uint64;

using f32 = float32;
using f64 = float64;

#define let const auto

//Reflection macros
#ifndef REFLECTION
	#define ENUM(...) enum class GAME_API
	#define COMPONENT(...) struct GAME_API
	#define SYSTEM(...) class GAME_API
	#define CLASS(...) class GAME_API
	#define PROPERTY(...)
#else
	#define ENUM(...) enum class __attribute__((annotate("enum:" #__VA_ARGS__)))
	#define COMPONENT(...) struct __attribute__((annotate("component:" #__VA_ARGS__)))
	#define SYSTEM(...) class __attribute__((annotate("system:" #__VA_ARGS__)))
	#define CLASS(...) class __attribute__((annotate("class:" #__VA_ARGS__)))
	#define PROPERTY(...) __attribute__((annotate("property:" #__VA_ARGS__)))
#endif

#include "Logger.h"
#include "Exceptions.h"
