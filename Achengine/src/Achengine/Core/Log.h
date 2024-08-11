#pragma once

#include "Core.h"
#include "spdlog/spdlog.h"
#include "spdlog/fmt/ostr.h"

namespace Achengine
{
	class ACHENGINE_API Log
	{
	public:

		static void Init();

		inline static std::shared_ptr<spdlog::logger>& GetCoreLogger() { return s_CoreLogger; }
		inline static std::shared_ptr<spdlog::logger>& GetClientLogger() { return s_ClientLogger; }

	private:

		static std::shared_ptr<spdlog::logger> s_ClientLogger;
		static std::shared_ptr<spdlog::logger> s_CoreLogger;

	};

}

// Core log macros
#define ACHENGINE_CORE_TRACE(...)		::Achengine::Log::GetCoreLogger()->trace(__VA_ARGS__)
#define ACHENGINE_CORE_INFO(...)		::Achengine::Log::GetCoreLogger()->info(__VA_ARGS__)
#define ACHENGINE_CORE_WARN(...)		::Achengine::Log::GetCoreLogger()->warn(__VA_ARGS__)
#define ACHENGINE_CORE_ERROR(...)		::Achengine::Log::GetCoreLogger()->error(__VA_ARGS__)
#define ACHENGINE_CORE_CRITICAL(...)	::Achengine::Log::GetCoreLogger()->critical(__VA_ARGS__)

// Client log macros
#define ACHENGINE_TRACE(...)			::Achengine::Log::GetClientLogger()->trace(__VA_ARGS__)
#define ACHENGINE_INFO(...)				::Achengine::Log::GetClientLogger()->info(__VA_ARGS__)
#define ACHENGINE_WARN(...)				::Achengine::Log::GetClientLogger()->warn(__VA_ARGS__)
#define ACHENGINE_ERROR(...)			::Achengine::Log::GetClientLogger()->error(__VA_ARGS__)
#define ACHENGINE_CRITICAL(...)			::Achengine::Log::GetClientLogger()->critical(__VA_ARGS__)