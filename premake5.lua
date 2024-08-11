workspace "Achengine"
	architecture "x64"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

-- Include directories relative to root folder (solution directory)
IncludeDir = {}
IncludeDir["GLFW"] = "Achengine/vendor/GLFW/include"
IncludeDir["Glad"] = "Achengine/vendor/Glad/include"
IncludeDir["ImGui"] = "Achengine/vendor/imgui"
IncludeDir["glm"] = "Achengine/vendor/glm"
IncludeDir["stb_image"] = "Achengine/vendor/stb_image"

include "Achengine/vendor/GLFW"
include "Achengine/vendor/Glad"
include "Achengine/vendor/imgui"

project "Achengine"
	location "Achengine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++14"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "Achenginepch.h"
	pchsource "Achengine/src/Achengine/Achenginepch.cpp"

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp",
		"%{prj.name}/vendor/stb_image/**.h",
		"%{prj.name}/vendor/stb_image/**.cpp",
		"%{prj.name}/vendor/glm/glm/**.hpp",
		"%{prj.name}/vendor/glm/glm/**.inl"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}
	
	includedirs
	{
		"%{prj.name}/src",
		"%{prj.name}/src/%{prj.name}",
		"%{prj.name}/vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"opengl32.lib"
	}

	filter "system:windows"
		staticruntime "On"
		systemversion "latest"

		defines
		{
			"ACHENGINE_PLATFORM_WINDOWS",
			"ACHENGINE_BUILD_DLL",
			"GLFW_INCLUDE_NONE"
		}

	filter "configurations:Debug"
		defines "ACHENGINE_DEBUG"
		staticruntime "off"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ACHENGINE_RELEASE"
		staticruntime "off"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ACHENGINE_DIST"
		staticruntime "off"
		runtime "Release"
		optimize "on"

project "Sandbox"
	location "Sandbox"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++14"
	staticruntime "on"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"%{prj.name}/src/**.h",
		"%{prj.name}/src/**.cpp"
	}
	
	includedirs
	{
		"Achengine/src",
		"Achengine/src/Achengine",
		"Achengine/vendor",
		"Achengine/vendor/spdlog/include",
		"%{IncludeDir.glm}"
	}

	links
	{
		"Achengine"
	}

	filter "system:windows"
		staticruntime "on"
		systemversion "latest"

		defines
		{
			"ACHENGINE_PLATFORM_WINDOWS",
		}

	filter "configurations:Debug"
		defines "ACHENGINE_DEBUG"
		staticruntime "off"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "ACHENGINE_RELEASE"
		staticruntime "off"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "ACHENGINE_DIST"
		staticruntime "off"
		runtime "Release"
		optimize "on"