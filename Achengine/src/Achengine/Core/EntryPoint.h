#pragma once

extern Achengine::Application* Achengine::CreateApplication();

	int main(int argc, char** argv)
	{
		Achengine::Log::Init();
		ACHENGINE_CORE_WARN("Initialized Log!");
		ACHENGINE_INFO("Hello!");

		auto app = Achengine::CreateApplication();
		app->Run();
		delete app;
	}