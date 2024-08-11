#pragma once

#include <memory>
#include <glm/glm.hpp>

#ifdef ACHENGINE_PLATFORM_WINDOWS
#if ACHENGINE_DYNAMIC_LINK
	#ifdef ACHENGINE_BUILD_DLL
		#define ACHENGINE_API _declspec(dllexport)
	#else
		#define ACHENGINE_API _declspec(dllimport)
	#endif
#else
	#define ACHENGINE_API
#endif
#else
	#error Achengine only supports Windows!
#endif

#ifdef ACHENGINE_ENABLE_ASSERTS
	#define ACHENGINE_ASSERT(x, ...) { if(!(x)) { ACHENGINE_ERROR("Assertion failed: {0}"), __VA_ARGS__); __debugbreak(); } }
	#define ACHENGINE_CORE_ASSERT(x, ...) { if(!(x)) { ACHENGINE_CORE_ERROR("Assertion failed: {0}"), __VA_ARGS__); __debugbreak(); } }
#else
	#define ACHENGINE_ASSERT(x, ...)
	#define ACHENGINE_CORE_ASSERT(x, ...)
#endif

#define BIT(x) (1 << x)

#define BIND_EVENT_FN(fn) std::bind(&fn, this, std::placeholders::_1)